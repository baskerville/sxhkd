VERSION = 0.1

CC      = gcc
LIBS    = -lm -lxcb -lxcb-keysyms
CFLAGS  = -std=c99 -pedantic -Wall -Wextra -DVERSION=\"$(VERSION)\"
LDFLAGS = $(LIBS)

PREFIX    ?= /usr/local
BINPREFIX = $(PREFIX)/bin

SRC = sxhkd.c keys.c helpers.c
OBJ = $(SRC:.c=.o)

all: CFLAGS += -Os
all: LDFLAGS += -s
all: options sxhkd

debug: CFLAGS += -O0 -g -DDEBUG
debug: options sxhkd

options:
	@echo "sxhkd build options:"
	@echo "CC      = $(CC)"
	@echo "CFLAGS  = $(CFLAGS)"
	@echo "LDFLAGS = $(LDFLAGS)"
	@echo "PREFIX  = $(PREFIX)"

.c.o:
	@echo "CC $<"
	@$(CC) $(CFLAGS) -DVERSION=\"$(VERSION)\" -c -o $@ $<

sxhkd: $(OBJ)
	@echo CC -o $@
	@$(CC) -o $@ $(OBJ) $(LDFLAGS)

clean:
	@echo "cleaning"
	@rm -f $(OBJ) sxhkd

install:
	@echo "installing executable files to $(DESTDIR)$(BINPREFIX)"
	@install -D -m 755 sxhkd $(DESTDIR)$(BINPREFIX)/xshkd

uninstall:
	@echo "removing executable files from $(DESTDIR)$(BINPREFIX)"
	@rm -f $(DESTDIR)$(BINPREFIX)/sxhkd

.PHONY: all debug options clean install uninstall
