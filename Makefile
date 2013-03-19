VERSION = 0.2

CC      = gcc
LIBS    = -lm -lxcb -lxcb-keysyms
CFLAGS  = -std=c99 -pedantic -Wall -Wextra -I$(PREFIX)/include
CFLAGS  += -D_POSIX_C_SOURCE=200112L -DVERSION=\"$(VERSION)\"
LDFLAGS = -L$(PREFIX)/lib

PREFIX    ?= /usr/local
BINPREFIX = $(PREFIX)/bin
MANPREFIX = $(PREFIX)/share/man

SRC = sxhkd.c hotkeys.c helpers.c
OBJ = $(SRC:.c=.o)

all: CFLAGS += -Os
all: LDFLAGS += -s
all: sxhkd

debug: CFLAGS += -O0 -g -DDEBUG
debug: sxhkd

include Incgraph

$(OBJ): Makefile

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

sxhkd: $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS) $(LIBS)

install:
	mkdir -p "$(DESTDIR)$(BINPREFIX)"
	cp sxhkd "$(DESTDIR)$(BINPREFIX)"
	chmod 755 "$(DESTDIR)$(BINPREFIX)/sxhkd"
	mkdir -p "$(DESTDIR)$(MANPREFIX)/man1"
	cp sxhkd.1 "$(DESTDIR)$(MANPREFIX)/man1"
	chmod 644 "$(DESTDIR)$(MANPREFIX)/man1/sxhkd.1"

uninstall:
	rm -f $(DESTDIR)$(BINPREFIX)/sxhkd
	rm -f $(DESTDIR)$(MANPREFIX)/man1/sxhkd.1

clean:
	rm -f $(OBJ) sxhkd

.PHONY: all debug clean install uninstall
