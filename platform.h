#ifndef UDU_PLATFORM_H
#define UDU_PLATFORM_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    bool is_directory;
    uint64_t size_apparent;
    uint64_t size_allocated;
} platform_stat_t;

typedef struct platform_dir platform_dir_t;

bool platform_stat(const char *path, platform_stat_t *st);
bool platform_is_directory(const char *path);
uint64_t platform_file_size(const char *path, bool apparent);

platform_dir_t *platform_opendir(const char *path);
const char *platform_readdir(platform_dir_t *dir);
void platform_closedir(platform_dir_t *dir);
bool is_symlink(const char *path);

#endif
