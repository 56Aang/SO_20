### Makefile ###

CC = gcc -g

all : argus server

argus: argus.c
	$(CC) argus.c functions.c $(CFLAGS) -o argus

server: server.c
	$(CC) server.c functions.c $(CFLAGS) -o server
	./server


compile:
	$(CC) server.c functions.c $(CFLAGS) -o server
	$(CC) argus.c functions.c $(CFLAGS) -o argus
	./server

.PHONY: run
run:
	$(CC) server.c functions.c $(CFLAGS) -o server
	$(CC) argus.c functions.c $(CFLAGS) -o argus
	./server


.PHONY: clean
clean:
	rm -f server
	rm -f argus

.PHONY: exit
exit:
	pkill -f server