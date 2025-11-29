#ifndef UDU_WALK_H
#define UDU_WALK_H

#include "args.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    uint64_t total_size;
    uint64_t nfiles;
    uint64_t ndirs;
} walk_result_t;

walk_result_t walk_paths(const args_t *cfg);

#endif
