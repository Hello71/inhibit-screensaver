prefix := /usr/local
bindir := $(prefix)/bin
CFLAGS += $(shell pkg-config --cflags dbus-1) -Wall -Wextra -std=c99
LDLIBS := $(shell pkg-config --libs dbus-1)

inhibit-screensaver: inhibit-screensaver.o

clean:
	$(RM) inhibit-screensaver.o inhibit-screensaver

install:
	install -D -m0755 inhibit-screensaver $(bindir)

uninstall:
	$(RM) $(bindir)/inhibit-screensaver
