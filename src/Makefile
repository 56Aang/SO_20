### Makefile ###

CC = gcc

all : server argus

server: server.c
	$(CC) server.c $(CFLAGS) -o server
	./server

argus: argus.c
	$(CC) argus.c $(CFLAGS) -o argus

compile:
	$(CC) server.c $(CFLAGS) -o server
	$(CC) argus.c $(CFLAGS) -o argus
	./server

.PHONY: run
run:
	$(CC) server.c $(CFLAGS) -o server
	$(CC) argus.c $(CFLAGS) -o argus
	./server


.PHONY: clean
clean:
	rm -f server
	rm -f argus

.PHONY: exit
exit:
	pkill -f server