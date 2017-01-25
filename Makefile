CC=gcc-6
CFLAGS=-std=c99 -Wall -O3 -D_DEFAULT_SOURCE-D_DEFAULT_SOURCE -fopenmp -fcilkplus

gameoflife: gameoflife.c
	$(CC) $(CFLAGS) -o gameoflife gameoflife.c

debug: CFLAGS += -DDEBUG -g
debug: gameoflife

clean:
	rm -f gameoflife
