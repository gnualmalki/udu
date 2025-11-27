# See COPYING for license details

.POSIX:

EXE       := udu
SRC       := args.c \
			 main.c \
			 platform.c \
			 util.c \
			 walk.c

OBJ       := $(SRC:.c=.o)
CC        := gcc
CFLAGS    := -Wall -Wextra -O3 -std=gnu11
LDFLAGS   :=
VERSION   := $(shell cat VERSION)
CFLAGS    += -DVERSION="\"$(VERSION)\""
DEPS      := $(OBJ:.o=.d)

all: options $(EXE)

# skip on non-build targets
ifeq ($(filter clean dist uninstall,$(MAKECMDGOALS)),)
    -include omp.mk
    -include lto.mk
endif

-include $(DEPS)

options:
	$(info [INFO]: CC = ($(CC)))
	$(info [INFO]: CFLAGS = ($(CFLAGS)))
	$(info [INFO]: LDFLAGS = ($(LDFLAGS)))

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(EXE): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

clean:
	rm -f $(EXE) $(OBJ) $(DEPS) ./*.tar.gz

dist: clean
	tar --exclude="*.tar.gz" -czf $(EXE)-$(VERSION).tar.gz .

install: $(EXE)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(EXE) $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/$(EXE)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(EXE)


.PHONY: all options clean dist install uninstall
