VERSION = 0.3
NAME = sxhkd

CC       = gcc
LIBS     = -lm -lxcb -lxcb-keysyms
CFLAGS  += -std=c99 -pedantic -Wall -Wextra -I$(PREFIX)/include
CFLAGS  += -D_POSIX_C_SOURCE=200112L -DVERSION=\"$(VERSION)\"
LDFLAGS += -L$(PREFIX)/lib

PREFIX    ?= /usr/local
BINPREFIX = $(PREFIX)/bin
MANPREFIX = $(PREFIX)/share/man

SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

all: CFLAGS += -Os
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
	mkdir -p "$(DESTDIR)$(MANPREFIX)/man1"
	cp -p $(NAME).1 "$(DESTDIR)$(MANPREFIX)/man1"

uninstall:
	rm -f $(DESTDIR)$(BINPREFIX)/$(NAME)
	rm -f $(DESTDIR)$(MANPREFIX)/man1/$(NAME).1

doc:
	pandoc -t json doc/README.md | runhaskell doc/capitalize_headers.hs | pandoc -f json -t man --template doc/man.template -V name=$(NAME) -o $(NAME).1
	pandoc -f markdown -t rst doc/README.md -o README.rst
	patch -p 1 -i doc/missed_emph.patch

clean:
	rm -f $(OBJ) sxhkd

.PHONY: all debug install uninstall doc clean
