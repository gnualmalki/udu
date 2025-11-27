# See COPYING for license details

.POSIX:

EXE       := udu
SRC       := source/args.c \
			 source/main.c \
			 source/platform.c \
			 source/util.c \
			 source/walk.c

OBJ       := $(SRC:.c=.o)
CC        := gcc
CFLAGS    := -Wall -Wextra -O3 -std=gnu11 -fopenmp
LDFLAGS   := -fopenmp
VERSION   := $(shell cat VERSION)
CFLAGS    += -DVERSION="\"$(VERSION)\""
DEPS      := $(OBJ:.o=.d)

# ifeq ($(MAC_CI),1)
# 	CFLAGS+= -I/usr/local/opt/libomp/include
# 	LDFLAGS+= -L/usr/local/opt/libomp/lib
# endif

all: options $(EXE)

options:
	@echo "CFLAGS  = $(CFLAGS)"
	@echo "LDFLAGS = $(LDFLAGS)"
	@echo "CC      = $(CC)"

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

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
