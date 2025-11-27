#include "walk.h"
#include <string.h>

#define MAX_SYMLINK_DEPTH 64
#define TREE_BRANCH "├── "
#define TREE_LAST "└── "
#define TREE_VERT "│   "
#define TREE_SPACE "    "
#define INIT_CHILD_CAPACITY 8

typedef struct tree_node_s
{
    char *name;
    uint64_t size;
    struct tree_node_s **children;
    uint16_t child_count;
    uint16_t child_capacity;
    bool is_dir;
} tree_node_t;

typedef struct
{
    char **excludes;
    int exclude_count;
    bool apparent_size;
    bool verbose;
    bool tree;
    uint64_t total_size;
    uint64_t file_count;
    uint64_t dir_count;
} walk_context_t;

static __thread char *tls_path_buf = NULL;
static __thread size_t tls_path_size = 0;

static char *get_path_buffer(size_t needed)
{
    if (needed > tls_path_size)
    {
        tls_path_size = (needed + 4095) & ~4095;
        tls_path_buf = realloc(tls_path_buf, tls_path_size);
    }
    return tls_path_buf;
}

static bool is_excluded(const char *name,
                        const char *fullpath,
                        const walk_context_t *ctx)
{
    for (int i = 0; i < ctx->exclude_count; i++)
    {
        if (glob_match(ctx->excludes[i], name) ||
            glob_match(ctx->excludes[i], fullpath))
            return true;
    }
    return false;
}

static tree_node_t *tree_node_create(const char *name,
                                     uint64_t size,
                                     bool is_dir)
{
    tree_node_t *node = malloc(sizeof(tree_node_t));
    node->name = strdup(name);
    node->size = size;
    node->is_dir = is_dir;
    node->child_count = 0;

    if (is_dir)
    {
        node->children = malloc(INIT_CHILD_CAPACITY * sizeof(tree_node_t *));
        node->child_capacity = INIT_CHILD_CAPACITY;
    }
    else
    {
        node->children = NULL;
        node->child_capacity = 0;
    }

    return node;
}

static void tree_node_add_child(tree_node_t *parent, tree_node_t *child)
{
    if (parent->child_count >= parent->child_capacity)
    {
        parent->child_capacity *= 2;
        parent->children = realloc(
          parent->children, parent->child_capacity * sizeof(tree_node_t *));
    }
    parent->children[parent->child_count++] = child;
}

static void tree_node_free(tree_node_t *node)
{
    if (!node) return;

    for (int i = 0; i < node->child_count; i++)
        tree_node_free(node->children[i]);

    free(node->name);
    free(node->children);
    free(node);
}

static int compare_nodes(const void *a, const void *b)
{
    tree_node_t *na = *(tree_node_t **)a;
    tree_node_t *nb = *(tree_node_t **)b;

    if (na->is_dir != nb->is_dir) return nb->is_dir - na->is_dir;

    return strcmp(na->name, nb->name);
}

static void print_tree_node(tree_node_t *node,
                            const char *prefix,
                            bool is_last,
                            const walk_context_t *ctx)
{
    const char *branch = is_last ? TREE_LAST : TREE_BRANCH;

    if (ctx->verbose)
    {
        char buf[32];
        printf("%s%s%-8s %s%s\n",
               prefix,
               branch,
               human_size(node->size, buf, sizeof(buf)),
               node->name,
               node->is_dir ? "/" : "");
    }
    else
    {
        printf(
          "%s%s%s%s\n", prefix, branch, node->name, node->is_dir ? "/" : "");
    }

    if (node->is_dir && node->child_count > 0)
    {
        const char *ext = is_last ? TREE_SPACE : TREE_VERT;
        size_t new_len = strlen(prefix) + 4;
        char *new_prefix = get_path_buffer(new_len + 1);

        snprintf(new_prefix, new_len + 1, "%s%s", prefix, ext);

        qsort(node->children,
              node->child_count,
              sizeof(tree_node_t *),
              compare_nodes);

        for (int i = 0; i < node->child_count; i++)
            print_tree_node(
              node->children[i], new_prefix, i == node->child_count - 1, ctx);
    }
}

static void process_file(uint64_t size, walk_context_t *ctx)
{
#pragma omp atomic
    ctx->total_size += size;
#pragma omp atomic
    ctx->file_count++;
}

static void process_file_verbose(const char *path,
                                 uint64_t size,
                                 walk_context_t *ctx)
{
    process_file(size, ctx);

#pragma omp critical(print)
    {
        char buf[32];
        printf("%-8s %s\n", human_size(size, buf, sizeof(buf)), path);
    }
}

static tree_node_t *build_tree_impl(const char *path,
                                    const char *name,
                                    walk_context_t *ctx,
                                    int depth)
{
    if (depth > MAX_SYMLINK_DEPTH) return NULL;

    platform_stat_t st;
    if (!platform_stat(path, &st)) return NULL;

    uint64_t size = ctx->apparent_size ? st.size_apparent : st.size_allocated;

    if (!st.is_directory)
    {
        process_file(size, ctx);
        return tree_node_create(name, size, false);
    }

#pragma omp atomic
    ctx->dir_count++;

    tree_node_t *node = tree_node_create(name, size, true);

    platform_dir_t *dir = platform_opendir(path);
    if (!dir) return node;

    size_t path_len = strlen(path);
    char *path_buf = get_path_buffer(path_len + 512);
    snprintf(path_buf, path_len + 2, "%s/", path);
    if (path_buf[path_len] == '/' && path_buf[path_len - 1] == '/')
        path_len--;
    else
        path_len++;

    const char *entry;
    while ((entry = platform_readdir(dir)) != NULL)
    {
        size_t full_len = path_len + strlen(entry);
        if (full_len + 1 > tls_path_size)
            path_buf = get_path_buffer(full_len + 1);

        snprintf(path_buf + path_len, tls_path_size - path_len, "%s", entry);

        if (is_excluded(entry, path_buf, ctx) || is_symlink(path_buf)) continue;

        char *fullpath = strdup(path_buf);
        char *entry_copy = strdup(entry);

#pragma omp task firstprivate(fullpath, entry_copy, depth) shared(ctx, node)
        {
            tree_node_t *child =
              build_tree_impl(fullpath, entry_copy, ctx, depth + 1);
            if (child)
            {
#pragma omp critical(tree_add)
                tree_node_add_child(node, child);
            }
            free(fullpath);
            free(entry_copy);
        }
    }

#pragma omp taskwait
    platform_closedir(dir);

    return node;
}

static void walk_directory_impl(const char *path,
                                walk_context_t *ctx,
                                int depth)
{
    if (depth > MAX_SYMLINK_DEPTH) return;

    platform_dir_t *dir = platform_opendir(path);
    if (!dir) return;

    size_t path_len = strlen(path);
    char *path_buf = get_path_buffer(path_len + 512);
    snprintf(path_buf, path_len + 2, "%s/", path);
    if (path_buf[path_len] == '/' && path_buf[path_len - 1] == '/')
        path_len--;
    else
        path_len++;

    const char *entry;
    while ((entry = platform_readdir(dir)) != NULL)
    {
        size_t full_len = path_len + strlen(entry);
        if (full_len + 1 > tls_path_size)
            path_buf = get_path_buffer(full_len + 1);

        snprintf(path_buf + path_len, tls_path_size - path_len, "%s", entry);

        if (is_excluded(entry, path_buf, ctx) || is_symlink(path_buf)) continue;

        platform_stat_t st;
        if (!platform_stat(path_buf, &st)) continue;

        char *fullpath = strdup(path_buf);

        if (st.is_directory)
        {
#pragma omp task firstprivate(fullpath, depth) shared(ctx)
            {
                walk_directory_impl(fullpath, ctx, depth + 1);
                free(fullpath);
            }
#pragma omp atomic
            ctx->dir_count++;
        }
        else
        {
            uint64_t size =
              ctx->apparent_size ? st.size_apparent : st.size_allocated;

            if (ctx->verbose)
                process_file_verbose(fullpath, size, ctx);
            else
                process_file(size, ctx);

            free(fullpath);
        }
    }

#pragma omp taskwait
    platform_closedir(dir);
}

walk_result_t walk_paths(char **paths,
                         int path_count,
                         char **excludes,
                         int exclude_count,
                         bool apparent_size,
                         bool verbose,
                         bool tree)
{
    walk_context_t ctx = { .excludes = excludes,
                           .exclude_count = exclude_count,
                           .apparent_size = apparent_size,
                           .verbose = verbose,
                           .tree = tree,
                           .total_size = 0,
                           .file_count = 0,
                           .dir_count = 0 };

#pragma omp parallel
#pragma omp single
    {
        for (int i = 0; i < path_count; i++)
        {
            const char *path = paths[i];

#pragma omp task firstprivate(path, i) shared(ctx)
            {
                platform_stat_t st;
                if (platform_stat(path, &st))
                {
                    if (tree)
                    {
                        const char *base_name = strrchr(path, '/');
                        base_name = base_name ? base_name + 1 : path;

                        tree_node_t *root =
                          st.is_directory
                            ? build_tree_impl(path, base_name, &ctx, 0)
                            : tree_node_create(base_name,
                                               ctx.apparent_size
                                                 ? st.size_apparent
                                                 : st.size_allocated,
                                               false);

                        if (!st.is_directory) process_file(root->size, &ctx);

#pragma omp critical(print)
                        {
                            printf("%s\n", path);
                            if (root->child_count > 0)
                            {
                                qsort(root->children,
                                      root->child_count,
                                      sizeof(tree_node_t *),
                                      compare_nodes);
                                for (int j = 0; j < root->child_count; j++)
                                    print_tree_node(root->children[j],
                                                    "",
                                                    j == root->child_count - 1,
                                                    &ctx);
                            }
                            tree_node_free(root);
                        }
                    }
                    else if (st.is_directory)
                    {
                        walk_directory_impl(path, &ctx, 0);
#pragma omp atomic
                        ctx.dir_count++;
                    }
                    else
                    {
                        uint64_t size =
                          apparent_size ? st.size_apparent : st.size_allocated;
                        if (verbose)
                            process_file_verbose(path, size, &ctx);
                        else
                            process_file(size, &ctx);
                    }
                }
                else
                {
#pragma omp critical(print)
                    fprintf(stderr, "Error: cannot stat '%s'\n", path);
                }
            }
        }
    }

    return (walk_result_t){ .total_size = ctx.total_size,
                            .file_count = ctx.file_count,
                            .dir_count = ctx.dir_count };
}
