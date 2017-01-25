CC=gcc-6
CFLAGS=-std=c99 -Wall -O3 -D_DEFAULT_SOURCE -fopenmp -fcilkplus

gameoflife: clean
gameoflife: gameoflife.c
	$(CC) $(CFLAGS) -o gameoflife gameoflife.c

debug: CFLAGS += -DDEBUG -g
debug: clean
debug: gameoflife

clean:
	rm -f gameoflife
