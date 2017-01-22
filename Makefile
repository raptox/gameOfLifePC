CC=gcc-4.9
CFLAGS=-std=c99 -Wall -O3 -D_DEFAULT_SOURCE-D_DEFAULT_SOURCE -fopenmp

gameoflife: gameoflife.c
	mkdir bin
	$(CC) $(CFLAGS) -o bin/gameoflife gameoflife.c

debug: CFLAGS += -DDEBUG -g
debug: gameoflife

clean:
	rm -rf bin
