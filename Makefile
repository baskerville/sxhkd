NAME    = sxhkd
VERSION = 0.4.1

CC       = gcc
LIBS     = -lm -lxcb -lxcb-keysyms
CFLAGS  += -std=c99 -pedantic -Wall -Wextra -I$(PREFIX)/include
CFLAGS  += -D_POSIX_C_SOURCE=200112L -DVERSION=\"$(VERSION)\"
LDFLAGS += -L$(PREFIX)/lib

PREFIX    ?= /usr/local
BINPREFIX  = $(PREFIX)/bin
MANPREFIX  = $(PREFIX)/share/man

SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

all: CFLAGS  += -Os
all: LDFLAGS += -s
all: $(NAME)

debug: CFLAGS += -O0 -g -DDEBUG
debug: $(NAME)

include Sourcedeps

$(OBJ): Makefile

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

$(NAME): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS) $(LIBS)

install:
	mkdir -p "$(DESTDIR)$(BINPREFIX)"
	cp -p $(NAME) "$(DESTDIR)$(BINPREFIX)"
	mkdir -p "$(DESTDIR)$(MANPREFIX)"/man1
	cp -p doc/$(NAME).1 "$(DESTDIR)$(MANPREFIX)"/man1

uninstall:
	rm -f "$(DESTDIR)$(BINPREFIX)"/$(NAME)
	rm -f "$(DESTDIR)$(MANPREFIX)"/man1/$(NAME).1

doc:
	a2x -v -d manpage -f manpage -a revnumber=$(VERSION) doc/$(NAME).1.txt

clean:
	rm -f $(OBJ) $(NAME)

.PHONY: all debug install uninstall doc clean
