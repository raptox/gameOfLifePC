/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>

/* prototypes */
void printUsage();
int lifeFunction(int nw, int n, int ne, int w, int c, int e, int sw, int s, int se);
void generateRandomGame(char ** gameMatrix, int m, int n);
void printGameMatrix(char ** gameMatrix, int m, int n);

/* main */
int main(int argc, char **argv) {
    // parse program options
    int option = 0;
    int m = -1, n = -1, g = -1;

    while ((option = getopt(argc, argv, "m:n:g:")) != -1) {
        switch (option) {
            case 'm': m = atoi(optarg); break;
            case 'n': n = atoi(optarg); break;
            case 'g': g = atoi(optarg); break;
            default: printUsage();
                exit(EXIT_FAILURE);
        }
    }

    if (m == -1 || n == -1 || g == -1) {
        printUsage();
        exit(EXIT_FAILURE);
    }

    // alloc game matrix
    char ** gameMatrix =(char **) malloc(m * sizeof(char *));
    for (int i = 0; i < n; i++) 
        gameMatrix[i] = (char *) malloc(n * sizeof(char));

    // begin game
    generateRandomGame(gameMatrix, m, n);
    for (int i = 0; i < g; i++) {
        printf("generation %d\n", i);
        printGameMatrix(gameMatrix, m, n);
        for (int x = 0; x < m; x++) {
            for (int y = 0; y < n; y++) {
                // do something
            }
        }
    }

    return EXIT_SUCCESS;
}

/* functions */
void printUsage() {
    printf("Usage: gameoflife -m num -n num -g num\n");
    printf("-m ... matrix width\n");
    printf("-n ... matrix height\n");
    printf("-g ... generations\n");
}

/**
 * here some detailed comments :)
 * 
 * . nw . n . ne 
 * .  w . c . e
 * . sw . s . se
 */
int lifeFunction(int nw, int n, int ne, int w, int c, int e, int sw, int s, int se) {


    return 0;
}

void generateRandomGame(char ** gameMatrix, int m, int n) {
    srand(time(NULL));
    for (int x = 0; x < m; x++) {
        for (int y = 0; y < n; y++) {
            gameMatrix[x][y] = rand() % 2;
        }
    }
}

void printGameMatrix(char ** gameMatrix, int m, int n) {
    for (int x = 0; x < m; x++) {
        for (int y = 0; y < n; y++) {
            printf("%d ", gameMatrix[x][y]);
        }
        printf("\n");
    }  
}