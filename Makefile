# See COPYING for license details

.POSIX:

EXE       := udu
MAN       := udu.1
SRC       := args.c \
             main.c \
             platform.c \
             util.c \
             walk.c

OBJ       := $(SRC:.c=.o)
DEPS      := $(OBJ:.o=.d)
CC        := gcc
CFLAGS    := -Wall -Wextra -O3 -std=gnu11
LDFLAGS   :=
VERSION   := $(shell cat VERSION)
CFLAGS    += -DVERSION="\"$(VERSION)\""
PREFIX    ?= /usr/local
BINDIR    := $(PREFIX)/bin
MANDIR    := $(PREFIX)/share/man/man1

all: options $(EXE)

# skip on non-build targets
ifeq ($(filter clean dist install uninstall,$(MAKECMDGOALS)),)
    -include omp.mk
    -include lto.mk
endif

-include $(DEPS)

options:
	$(info [INFO]: CC = $(CC))
	$(info [INFO]: CFLAGS = $(CFLAGS))
	$(info [INFO]: LDFLAGS = $(LDFLAGS))

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(EXE): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

clean:
	rm -f $(EXE) $(OBJ) $(DEPS) ./*.tar.gz

dist: clean
	tar --exclude="*.tar.gz" -czf $(EXE)-$(VERSION).tar.gz .

install: $(EXE)
	mkdir -p $(DESTDIR)$(BINDIR)
	cp -f $(EXE) $(DESTDIR)$(BINDIR)
	chmod 755 $(DESTDIR)$(BINDIR)/$(EXE)

	mkdir -p $(DESTDIR)$(MANDIR)
	cp -f $(MAN) $(DESTDIR)$(MANDIR)
	chmod 644 $(DESTDIR)$(MANDIR)/$(MAN)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(EXE)
	rm -f $(DESTDIR)$(MANDIR)/$(MAN)


.PHONY: all options clean dist install uninstall
