#include "walk.h"
#include "args.h"
#include "const.h"
#include "platform.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _OPENMP
    #include <omp.h>
#endif

#define MAX_DEPTH 64
#define BRANCH "├── "
#define LAST "└── "
#define VERT "│   "
#define SPACE "    "
#define INIT_CAP 64

typedef struct node_s
{
    char *name;
    uint64_t size;
    struct node_s **kids;
    uint16_t nkids;
    uint16_t cap;
    bool dir;
} node_t;

typedef struct
{
    char **excl;
    int nexcl;
    bool apparent;
    bool verbose;
    bool tree;
    uint64_t size;
    uint64_t nfiles;
    uint64_t ndirs;
} ctx_t;

static UDU_THD char *buf = NULL;
static UDU_THD size_t bufsz = 0;

UDU_SI char *getbuf(size_t needed)
{
    if (needed > bufsz)
    {
        bufsz = (needed + 4095) & ~4095;
        buf = realloc(buf, bufsz);
    }
    return buf;
}

UDU_SI bool is_excluded(const char *name, const char *path, const ctx_t *ctx)
{
    for (int i = 0; i < ctx->nexcl; i++)
    {
        if (glob_match(ctx->excl[i], name) || glob_match(ctx->excl[i], path))
            return true;
    }
    return false;
}

UDU_SI node_t *mk_node(const char *name, uint64_t size, bool dir)
{
    node_t *node = malloc(sizeof(node_t));
    node->name = strdup(name);
    node->size = size;
    node->dir = dir;
    node->nkids = 0;
    node->cap = dir ? INIT_CAP : 0;
    node->kids = dir ? malloc(INIT_CAP * sizeof(node_t *)) : NULL;
    return node;
}

UDU_SI void node_add(node_t *parent, node_t *child)
{
    if (parent->nkids >= parent->cap)
    {
        parent->cap *= 2;
        parent->kids = realloc(parent->kids, parent->cap * sizeof(node_t *));
    }
    parent->kids[parent->nkids++] = child;
}

void node_free(node_t *node)
{
    if (!node) return;
    for (int i = 0; i < node->nkids; i++) node_free(node->kids[i]);
    free(node->name);
    free(node->kids);
    free(node);
}

UDU_SI int node_cmp(const void *a, const void *b)
{
    node_t *node_a = *(node_t **)a;
    node_t *node_b = *(node_t **)b;
    if (node_a->dir != node_b->dir) return node_b->dir - node_a->dir;
    return strcmp(node_a->name, node_b->name);
}

uint64_t calc_total_size(node_t *node)
{
    if (!node->dir) return node->size;

    uint64_t total = node->size;
    for (int i = 0; i < node->nkids; i++)
        total += calc_total_size(node->kids[i]);

    return total;
}

static void print(node_t *node,
                  const char *prefix,
                  bool is_last,
                  const ctx_t *ctx)
{
    const char *branch = is_last ? LAST : BRANCH;

    if (ctx->verbose)
    {
        char sizebuf[32];
        uint64_t display_size = node->dir ? calc_total_size(node) : node->size;
        printf("%s%s%-8s %s%s\n",
               prefix,
               branch,
               human_size(display_size, sizebuf, sizeof(sizebuf)),
               node->name,
               node->dir ? "/" : "");
    }
    else
    {
        printf("%s%s%s%s\n", prefix, branch, node->name, node->dir ? "/" : "");
    }

    if (node->dir && node->nkids > 0)
    {
        const char *extension = is_last ? SPACE : VERT;
        size_t prefix_len = strlen(prefix) + 4;
        char *new_prefix = getbuf(prefix_len + 1);
        snprintf(new_prefix, prefix_len + 1, "%s%s", prefix, extension);

        for (int i = 0; i < node->nkids; i++)
            print(node->kids[i], new_prefix, i == node->nkids - 1, ctx);
    }
}

UDU_SI void record_file(uint64_t size, ctx_t *ctx)
{
#ifdef _OPENMP
    #pragma omp atomic
#endif
    ctx->size += size;
#ifdef _OPENMP
    #pragma omp atomic
#endif
    ctx->nfiles++;
}

UDU_SI void record_verbose(const char *path, uint64_t size, ctx_t *ctx)
{
    record_file(size, ctx);
#ifdef _OPENMP
    #pragma omp critical(print)
#endif
    {
        char sizebuf[32];
        printf("%-8s %s\n", human_size(size, sizebuf, sizeof(sizebuf)), path);
    }
}

static node_t *mk_tree(const char *path,
                       const char *name,
                       ctx_t *ctx,
                       int depth)
{
    if (depth > MAX_DEPTH) return NULL;

    platform_stat_t st;
    if (!platform_stat(path, &st)) return NULL;

    uint64_t size = ctx->apparent ? st.size_apparent : st.size_allocated;

    if (!st.is_directory)
    {
        record_file(size, ctx);
        return mk_node(name, size, false);
    }

#ifdef _OPENMP
    #pragma omp atomic
#endif
    ctx->ndirs++;

    node_t *node = mk_node(name, size, true);
    platform_dir_t *dir = platform_opendir(path);
    if (!dir) return node;

    size_t path_len = strlen(path);
    char *pathbuf = getbuf(path_len + 512);

    if (path_len > 0 && path[path_len - 1] == '/')
    {
        memcpy(pathbuf, path, path_len);
        pathbuf[path_len] = '\0';
    }
    else
    {
        memcpy(pathbuf, path, path_len);
        pathbuf[path_len] = '/';
        pathbuf[path_len + 1] = '\0';
        path_len++;
    }

    const char *entry;
    while ((entry = platform_readdir(dir)))
    {
        size_t entry_len = strlen(entry);
        size_t full_len = path_len + entry_len;
        if (full_len + 1 > bufsz) pathbuf = getbuf(full_len + 1);

        memcpy(pathbuf + path_len, entry, entry_len + 1);

        if (is_excluded(entry, pathbuf, ctx) || is_symlink(pathbuf)) continue;

        char *fullpath = strdup(pathbuf);
        char *entry_copy = strdup(entry);

#ifdef _OPENMP
    #pragma omp task firstprivate(fullpath, entry_copy, depth) shared(ctx, node)
#endif
        {
            node_t *child = mk_tree(fullpath, entry_copy, ctx, depth + 1);
            if (child)
            {
#ifdef _OPENMP
    #pragma omp critical(tree_add)
#endif
                node_add(node, child);
            }
            free(fullpath);
            free(entry_copy);
        }
    }

#ifdef _OPENMP
    #pragma omp taskwait
#endif
    platform_closedir(dir);
    return node;
}

static void walk(const char *path, ctx_t *ctx, int depth)
{
    if (depth > MAX_DEPTH) return;

    platform_dir_t *dir = platform_opendir(path);
    if (!dir) return;

    size_t path_len = strlen(path);
    char *pathbuf = getbuf(path_len + 512);

    if (path_len > 0 && path[path_len - 1] == '/')
    {
        memcpy(pathbuf, path, path_len);
        pathbuf[path_len] = '\0';
    }
    else
    {
        memcpy(pathbuf, path, path_len);
        pathbuf[path_len] = '/';
        pathbuf[path_len + 1] = '\0';
        path_len++;
    }

    const char *entry;
    while ((entry = platform_readdir(dir)))
    {
        size_t entry_len = strlen(entry);
        size_t full_len = path_len + entry_len;
        if (full_len + 1 > bufsz) pathbuf = getbuf(full_len + 1);

        memcpy(pathbuf + path_len, entry, entry_len + 1);

        if (is_excluded(entry, pathbuf, ctx) || is_symlink(pathbuf)) continue;

        platform_stat_t st;
        if (!platform_stat(pathbuf, &st)) continue;

        char *fullpath = strdup(pathbuf);

        if (st.is_directory)
        {
#ifdef _OPENMP
    #pragma omp task firstprivate(fullpath, depth) shared(ctx)
#endif
            {
                walk(fullpath, ctx, depth + 1);
                free(fullpath);
            }
#ifdef _OPENMP
    #pragma omp atomic
#endif
            ctx->ndirs++;
        }
        else
        {
            uint64_t size =
              ctx->apparent ? st.size_apparent : st.size_allocated;
            if (ctx->verbose)
                record_verbose(fullpath, size, ctx);
            else
                record_file(size, ctx);
            free(fullpath);
        }
    }

#ifdef _OPENMP
    #pragma omp taskwait
#endif
    platform_closedir(dir);
}

walk_result_t walk_paths(const args_t *cfg)
{
    ctx_t ctx = { .excl = cfg->excludes,
                  .nexcl = cfg->exclude_count,
                  .apparent = cfg->apparent_size,
                  .verbose = cfg->verbose,
                  .tree = cfg->tree,
                  .size = 0,
                  .nfiles = 0,
                  .ndirs = 0 };

#ifdef _OPENMP
    #pragma omp parallel
    #pragma omp single
#endif
    {
        for (int i = 0; i < cfg->path_count; i++)
        {
            const char *path = cfg->paths[i];

#ifdef _OPENMP
    #pragma omp task firstprivate(path, i) shared(ctx)
#endif
            {
                platform_stat_t st;
                if (!platform_stat(path, &st))
                {
#ifdef _OPENMP
    #pragma omp critical(print)
#endif
                    {
                        fprintf(stderr, "Error: cannot stat '%s'\n", path);
                    }
                }
                else if (ctx.tree)
                {
                    const char *basename = strrchr(path, '/');
                    basename = basename ? basename + 1 : path;

                    node_t *root = NULL;

                    if (st.is_directory)
                    {
                        root = mk_tree(path, basename, &ctx, 0);
                    }
                    else
                    {
                        uint64_t size =
                          ctx.apparent ? st.size_apparent : st.size_allocated;
                        root = mk_node(basename, size, false);
                        record_file(size, &ctx);
                    }

#ifdef _OPENMP
    #pragma omp critical(print)
#endif
                    {
                        if (root)
                        {
                            if (root->nkids > 0)
                            {
                                qsort(root->kids,
                                      root->nkids,
                                      sizeof(node_t *),
                                      node_cmp);
                            }

                            if (ctx.verbose)
                            {
                                char sizebuf[32];
                                uint64_t display_size =
                                  root->dir ? calc_total_size(root)
                                            : root->size;
                                printf("%s %-8s\n",
                                       path,
                                       human_size(display_size,
                                                  sizebuf,
                                                  sizeof(sizebuf)));
                            }
                            else
                            {
                                printf("%s\n", path);
                            }

                            if (root->nkids > 0)
                            {
                                for (int j = 0; j < root->nkids; j++)
                                    print(root->kids[j],
                                          "",
                                          j == root->nkids - 1,
                                          &ctx);
                            }
                            node_free(root);
                        }
                    }
                }
                else if (st.is_directory)
                {
                    walk(path, &ctx, 0);
#ifdef _OPENMP
    #pragma omp atomic
#endif
                    ctx.ndirs++;
                }
                else
                {
                    uint64_t size =
                      ctx.apparent ? st.size_apparent : st.size_allocated;
                    if (ctx.verbose)
                        record_verbose(path, size, &ctx);
                    else
                        record_file(size, &ctx);
                }
            }
        }
    }

    return (walk_result_t){ .total_size = ctx.size,
                            .nfiles = ctx.nfiles,
                            .ndirs = ctx.ndirs };
}
