VERSION = 0.1

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
	@$(CC) $(CFLAGS) -c -o $@ $<

sxhkd: $(OBJ)
	@echo CC -o $@
	@$(CC) -o $@ $(OBJ) $(LDFLAGS) $(LIBS)

clean:
	@echo "cleaning"
	@rm -f $(OBJ) sxhkd

install:
	@echo "installing executable files to $(DESTDIR)$(BINPREFIX)"
	@mkdir -p "$(DESTDIR)$(BINPREFIX)"
	@cp sxhkd "$(DESTDIR)$(BINPREFIX)"
	@chmod 755 "$(DESTDIR)$(BINPREFIX)/sxhkd"
	@echo "installing manual page to $(DESTDIR)$(MANPREFIX)/man1"
	@mkdir -p "$(DESTDIR)$(MANPREFIX)/man1"
	@cp sxhkd.1 "$(DESTDIR)$(MANPREFIX)/man1"
	@chmod 644 "$(DESTDIR)$(MANPREFIX)/man1/sxhkd.1"

uninstall:
	@echo "removing executable files from $(DESTDIR)$(BINPREFIX)"
	@rm -f $(DESTDIR)$(BINPREFIX)/sxhkd
	@echo "removing manual page from $(DESTDIR)$(MANPREFIX)/man1"
	@rm -f $(DESTDIR)$(MANPREFIX)/man1/sxhkd.1

.PHONY: all debug options clean install uninstall
