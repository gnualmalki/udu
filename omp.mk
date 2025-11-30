#
# check if openmp version met the requirements
#
OMP_VER := $(shell echo '#include <omp.h>' | $(CC) -x c - -dM -E -fopenmp 2>/dev/null | awk '/_OPENMP/ {print $$3}')

ifeq ($(OMP_VER),)
    OMP_MSG := disabled (OpenMP < 3.0 detected or not available)
else ifneq ($(shell [ $(OMP_VER) -ge 200805 ] && echo OK),)
    CFLAGS  += -fopenmp
    LDFLAGS += -fopenmp
    OMP_MSG := enabled (version: $(OMP_VER))
else
    OMP_MSG := disabled (OpenMP < 3.0 detected or not available)
endif
