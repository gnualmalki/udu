#
# check if openmp version met the requirements
#
OMP_OK := $(shell echo '\#include <omp.h>' | \
           $(CC) -x c - -fopenmp -dM -E 2>/dev/null | \
           awk '/_OPENMP/ { if ($$3 >= 200805) print "OK" }')

ifeq ($(OMP_OK),OK)
    $(info [INFO]: OpenMP version >= 3.0 found (enabled))
    CFLAGS  += -fopenmp
    LDFLAGS += -fopenmp
else
	$(warning [WARNING]: OpenMP < 3.0 detected or not available (BUILDING WITHOUT OPENMP))
endif
