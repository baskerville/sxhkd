OUT      = sxhkd
VERCMD  ?= git describe --tags 2> /dev/null
VERSION := $(shell $(VERCMD) || cat VERSION)

CPPFLAGS += -D_POSIX_C_SOURCE=200112L -DVERSION=\"$(VERSION)\"
CFLAGS   += -std=c99 -pedantic -Wall -Wextra
LDFLAGS  ?=
LDLIBS    = $(LDFLAGS) -lxcb -lxcb-keysyms

PREFIX    ?= /usr/local
BINPREFIX ?= $(PREFIX)/bin
MANPREFIX ?= $(PREFIX)/share/man
DOCPREFIX ?= $(PREFIX)/share/doc/$(OUT)

all: $(OUT)

debug: CFLAGS += -O0 -g
debug: CPPFLAGS += -DDEBUG
debug: $(OUT)

VPATH = src
OBJ   =

include Sourcedeps

$(OBJ): Makefile

$(OUT): $(OBJ)

install:
	mkdir -p "$(DESTDIR)$(BINPREFIX)"
	cp -pf $(OUT) "$(DESTDIR)$(BINPREFIX)"
	mkdir -p "$(DESTDIR)$(MANPREFIX)"/man1
	cp -p doc/$(OUT).1 "$(DESTDIR)$(MANPREFIX)"/man1
	mkdir -p "$(DESTDIR)$(DOCPREFIX)"
	cp -pr examples "$(DESTDIR)$(DOCPREFIX)"/examples

uninstall:
	rm -f "$(DESTDIR)$(BINPREFIX)"/$(OUT)
	rm -f "$(DESTDIR)$(MANPREFIX)"/man1/$(OUT).1
	rm -rf "$(DESTDIR)$(DOCPREFIX)"

doc:
	a2x -v -d manpage -f manpage -a revnumber=$(VERSION) doc/$(OUT).1.asciidoc

clean:
	rm -f $(OBJ) $(OUT)

.PHONY: all debug install uninstall doc clean
