NAME     = sxhkd
VERCMD  ?= git describe 2> /dev/null
VERSION := $(shell $(VERCMD) || cat VERSION)

CPPFLAGS += -D_POSIX_C_SOURCE=200112L -DVERSION=\"$(VERSION)\"
CFLAGS   += -std=c99 -pedantic -Wall -Wextra
LDLIBS    = -lxcb -lxcb-keysyms

PREFIX    ?= /usr/local
BINPREFIX ?= $(PREFIX)/bin
MANPREFIX ?= $(PREFIX)/share/man
DOCPREFIX ?= $(PREFIX)/share/doc/$(NAME)

SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)

all: $(NAME)

debug: CFLAGS += -O0 -g
debug: CPPFLAGS += -DDEBUG
debug: $(NAME)

include Sourcedeps

$(OBJ): Makefile

$(NAME): $(OBJ)

install:
	mkdir -p "$(DESTDIR)$(BINPREFIX)"
	cp -pf $(NAME) "$(DESTDIR)$(BINPREFIX)"
	mkdir -p "$(DESTDIR)$(MANPREFIX)"/man1
	cp -p doc/$(NAME).1 "$(DESTDIR)$(MANPREFIX)"/man1
	mkdir -p "$(DESTDIR)$(DOCPREFIX)"
	cp -pr examples "$(DESTDIR)$(DOCPREFIX)"/examples

uninstall:
	rm -f "$(DESTDIR)$(BINPREFIX)"/$(NAME)
	rm -f "$(DESTDIR)$(MANPREFIX)"/man1/$(NAME).1
	rm -rf "$(DESTDIR)$(DOCPREFIX)"

doc:
	a2x -v -d manpage -f manpage -a revnumber=$(VERSION) doc/$(NAME).1.asciidoc

clean:
	rm -f $(OBJ) $(NAME)

.PHONY: all debug install uninstall doc clean
