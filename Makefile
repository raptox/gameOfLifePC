CC=gcc-4.9
CFLAGS=-std=c99 -Wall -O3

gameoflife: gameoflife.c
	$(CC) $(CFLAGS) -o gameoflife gameoflife.c  

clean:
	rm -f gameoflife.o
