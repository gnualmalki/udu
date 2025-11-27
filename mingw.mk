#
# MinGW toolchain
#
# Usage: make -f Makefile -f mingw.mk

CC        := x86_64-w64-mingw32-gcc
LDFLAGS   += -static -static-libgcc
EXE       := $(EXE).exe
