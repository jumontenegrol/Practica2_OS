CC := gcc
GTK_CFLAGS := $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS := $(shell pkg-config --libs gtk+-3.0)
CFLAGS := -Wall -Wextra


BINS := gui2 back_sockets indexer


.PHONY: all build run server clean run-indexer


all: build


build: gui2 back_sockets indexer


gui2: gui2.c
	$(CC) $(GTK_CFLAGS) $(CFLAGS) gui2.c -o gui2 $(GTK_LIBS)


back_sockets: back_sockets.c hashmap.c
	$(CC) $(CFLAGS) back_sockets.c hashmap.c -o back_sockets


indexer: indexer.c hashmap.c
	$(CC) $(CFLAGS) indexer.c hashmap.c -o indexer


# Run the GUI
run: gui2
	./gui2


# Run the backend/server
server: back_sockets
	./back_sockets


# Run indexer (separate target in case you want to execute it after build)
run-indexer: indexer
	./indexer


clean:
	rm -f *.o *.data $(BINS)