#ifndef UDU_WALK_H
#define UDU_WALK_H

#include "platform.h"
#include "util.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _OPENMP
    #include <omp.h>
#endif

typedef struct
{
    uint64_t total_size;
    uint64_t file_count;
    uint64_t dir_count;
} walk_result_t;

walk_result_t walk_paths(char **paths,
                         int path_count,
                         char **excludes,
                         int exclude_count,
                         bool apparent_size,
                         bool verbose);

#endif
