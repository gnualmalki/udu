#
# check if openmp version met the requirements
#
OMP_OK := $(shell echo '\#include <omp.h>' | \
           $(CC) -x c - -fopenmp -dM -E 2>/dev/null | \
           awk '/_OPENMP/ { if ($$3 >= 200805) print "OK" }')

ifeq ($(OMP_OK),OK)
    CFLAGS  += -fopenmp
    LDFLAGS += -fopenmp
	OMP_MSG := enabled (OpenMP >= 3.0)
else
    OMP_MSG := disabled (OpenMP < 3.0 detected or not available)
endif
