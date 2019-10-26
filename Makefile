.POSIX:
CFLAGS = --std=c99 -O3 -Wall -Wextra
LDFLAGS = -lm
PREFIX = /usr/local
main: main.c
	$(CC) $(CFLAGS) -Iinclude/ src/generation/* src/processing/* \
                        src/tools/* src/macros/* src/parser.c src/utils.c \
                        main.c -o sunset $(LDFLAGS)

all: main

clean: 
	rm -f sunset

install:
	cp sunset $(DESTDIR)$(PREFIX)/bin/

uninstall:
	rm $(DESTDIR)$(PREFIX)/bin/sunset

PHONY = all clean install uninstall
