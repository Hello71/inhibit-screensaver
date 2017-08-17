prefix := /usr/local
bindir := $(prefix)/bin
CFLAGS += $(shell pkg-config --cflags dbus-1) -Wall -Wextra -std=c99
LDLIBS := $(shell pkg-config --libs dbus-1)

all: inhibit-screensaver c-example
inhibit-screensaver: inhibit-screensaver.o

c-example: c-example.o

c-example.o: example.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

clean:
	$(RM) inhibit-screensaver.o inhibit-screensaver c-example.o c-example

install:
	install -D -m0755 inhibit-screensaver $(bindir)

uninstall:
	$(RM) $(bindir)/inhibit-screensaver

.PHONY: clean install uninstall
