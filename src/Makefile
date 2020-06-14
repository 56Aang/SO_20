### Makefile ###

CC = gcc -g

all : argus server fifo

argus: argus.c
	$(CC) argus.c $(CFLAGS) -o argus

server: argusd.c
	$(CC) argusd.c functions.c $(CFLAGS) -o argusd

fifo: mkfifo.c
	$(CC) mkfifo.c $(CFLAGS) -o mkfifo
	./mkfifo

compile:
	$(CC) argusd.c functions.c $(CFLAGS) -o argusd
	$(CC) argus.c functions.c $(CFLAGS) -o argus
	$(CC) mkfifo.c $(CFLAGS) -o mkfifo

.PHONY: run
run:
	$(CC) argusd.c functions.c $(CFLAGS) -o argusd
	$(CC) argus.c functions.c $(CFLAGS) -o argus
	./argusd


.PHONY: clean
clean:
	rm -f argusd
	rm -f argus
	rm -f mkfifo
	rm -f pipe*
	rm -f fifo-sv-cl
	rm -f fifo-cl-sv

.PHONY: exit
exit:
	pkill -f argusd