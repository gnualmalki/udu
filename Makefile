# See COPYING for license details

.POSIX:

EXE       := udu
MAN       := udu.1
SRC       := main.c walk.c
             

OBJ       := $(SRC:.c=.o)
DEPS      := $(OBJ:.o=.d)
CC        := cc
CFLAGS    := -Wall -Wextra -O3 -std=gnu11
LDFLAGS   :=
VERSION   := $(shell cat VERSION)
CFLAGS    += -DVERSION="\"$(VERSION)\""
PREFIX    ?= /usr/local
BINDIR    := $(PREFIX)/bin
MANDIR    := $(PREFIX)/share/man/man1

all: options $(EXE)

# skip non-build targets
ifeq ($(filter clean dist install uninstall,$(MAKECMDGOALS)),)
    -include omp.mk
    -include lto.mk
endif

-include $(DEPS)

options:
	@echo "[INFO]: CC = $(CC)"
	@echo "[INFO]: CFLAGS = $(CFLAGS)"
	@echo "[INFO]: LDFLAGS = $(LDFLAGS)"
	@echo "[INFO]: OPENMP = $(OMP_MSG)"
	@echo "[INFO]: LTO = $(LTO_MSG)"

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(EXE): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

clean:
	rm -f $(EXE) $(OBJ) $(DEPS) ./*.tar.gz

dist: clean
	tar --exclude="*.tar.gz" -czf $(EXE)-$(VERSION).tar.gz .

install: $(EXE)
	install -Dv -m 755 $(EXE) $(BINDIR)/$(EXE)
	install -Dv -m 644 $(MAN) $(MANDIR)/$(MAN)

uninstall:
	rm -f $(BINDIR)/$(EXE)
	rm -f $(MANDIR)/$(MAN)

man:
	rm -f udu.1
	pandoc -f markdown -s -t man udu.man -o udu.1


.PHONY: all options clean dist install uninstall
