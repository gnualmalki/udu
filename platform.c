#include "platform.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define BLOCK_SIZE 512

struct platform_dir
{
    DIR *dir;
    struct dirent *entry;
};

bool platform_stat(const char *path, platform_stat_t *st)
{
    struct stat sb;
    if (stat(path, &sb) != 0)
    {
        return false;
    }

    st->is_directory = S_ISDIR(sb.st_mode);
    st->size_apparent = (uint64_t)sb.st_size;

#if defined(__APPLE__) || defined(__linux__) // BSDs....??
    st->size_allocated = (uint64_t)sb.st_blocks * BLOCK_SIZE;
#else
    st->size_allocated = st->size_apparent;
#endif

    return true;
}

bool platform_is_directory(const char *path)
{
    struct stat sb;
    return stat(path, &sb) == 0 && S_ISDIR(sb.st_mode);
}

uint64_t platform_file_size(const char *path, bool apparent)
{
    platform_stat_t st;
    return platform_stat(path, &st)
             ? (apparent ? st.size_apparent : st.size_allocated)
             : 0;
}

platform_dir_t *platform_opendir(const char *path)
{
    platform_dir_t *dir = malloc(sizeof(platform_dir_t));
    if (!dir) return NULL;

    dir->dir = opendir(path);
    if (!dir->dir)
    {
        free(dir);
        return NULL;
    }

    dir->entry = NULL;
    return dir;
}

const char *platform_readdir(platform_dir_t *dir)
{
    if (!dir || !dir->dir) return NULL;

    while ((dir->entry = readdir(dir->dir)) != NULL)
    {
        const char *name = dir->entry->d_name;
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
        {
            continue;
        }
        return name;
    }

    return NULL;
}

void platform_closedir(platform_dir_t *dir)
{
    if (dir)
    {
        if (dir->dir) closedir(dir->dir);
        free(dir);
    }
}

bool is_symlink(const char *path)
{
    struct stat st;
    if (lstat(path, &st) != 0) return false;
    return S_ISLNK(st.st_mode);
}
