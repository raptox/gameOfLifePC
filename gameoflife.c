/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>

#include <omp.h>

/* prototypes */
void printUsage();
char lifeFunction(int nw, int n, int ne, int w, int c, int e, int sw, int s, int se);
void generateRandomGame(char ** gameMatrix, int m, int n);
void printGameMatrix(char ** gameMatrix, int m, int n, int flag);

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
    int flag = 0;
    // neighbours
    int nw, no, ne, w, c, e, sw, s, se;

    for (int i = 0; i < g; i++) {
        #if DEBUG
        printf("generation %d\n", i);
        printGameMatrix(gameMatrix, m, n, flag);
        #endif
        //#pragma omp parallel for
        for (int x = 0; x < m; x++) {
            for (int y = 0; y < n; y++) {
              nw = (x == 0 || y == 0) ? 0 : (gameMatrix[x-1][y-1] & (flag+1)) > 0;
              no = (x == 0) ? 0 : (gameMatrix[x-1][y] & (flag+1)) > 0;
              ne = (x == 0 || y == n-1) ? 0 : (gameMatrix[x-1][y+1] & (flag+1)) > 0;
              w = (y == 0) ? 0 : (gameMatrix[x][y-1] & (flag+1)) > 0;
              c = (gameMatrix[x][y] & (flag+1)) > 0;
              e = (y == n-1) ? 0 : (gameMatrix[x][y+1] & (flag+1)) > 0;
              sw = (x == m-1 || y == 0) ? 0 : (gameMatrix[x+1][y-1] & (flag+1)) > 0;
              s = (x == m-1) ? 0 : (gameMatrix[x+1][y] & (flag+1)) > 0;
              se = (x == m-1 || y == n-1) ? 0 : (gameMatrix[x+1][y+1] & (flag+1)) > 0;
              //printf("nw %d, n %d, ne %d, w %d, c %d, e %d, sw %d, s %d, se %d\n", nw, no, ne, w, c, e, sw, s, se);
              gameMatrix[x][y] = (gameMatrix[x][y] % 2) + (lifeFunction(nw, no, ne, w, c, e, sw, s, se) << !flag);
            }
        }
        flag = !flag;
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
char lifeFunction(int nw, int n, int ne, int w, int c, int e, int sw, int s, int se) {
    int living = nw + n + ne + w + e + sw + s + se;
    if (c == 0) {
      if (living == 3) return 1;
      return 0;
    }
    else {
      if (living < 2) return 0;
      if (living > 3) return 0;
      return 1;
    }
}

void generateRandomGame(char ** gameMatrix, int m, int n) {
    //srand(time(NULL));
    for (int x = 0; x < m; x++) {
        for (int y = 0; y < n; y++) {
            gameMatrix[x][y] = rand() % 2;
        }
    }
}

void printGameMatrix(char ** gameMatrix, int m, int n, int flag) {
    for (int x = 0; x < m; x++) {
        for (int y = 0; y < n; y++) {
            printf("%d ", (gameMatrix[x][y] & (flag+1)) > 0);
        }
        printf("\n");
    }
}
