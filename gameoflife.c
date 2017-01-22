/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <omp.h>

/* prototypes */
void printUsage();
unsigned char lifeFunction(int nw, int n, int ne, int w, int c, int e, int sw, int s, int se);
void generateRandomGame(unsigned char ** gameMatrix, int width, int height);
void printGameMatrix(unsigned char ** gameMatrix, int width, int height, unsigned char switchValuesFlag);
void runGame(int width, int height, int generations, int mode);
unsigned char ** allocateGameSpace(int width, int height);
void evolve_normal(unsigned char ** gameMatrix, int width, int height, unsigned char switchValuesFlag);
void evolve_omp(unsigned char ** gameMatrix, int width, int height, unsigned char switchValuesFlag);
void parseProgramOptions(int argc, char **argv, int * width, int * height, int * generations, int * mode);
unsigned char setValuesCode(unsigned char currentCode, int alive, unsigned char switchValuesFlag);

/* defines */
#define for_x for (int x = 0; x < width; x++)
#define for_y for (int y = 0; y < height; y++)
#define for_xy for_x for_y

/* main */
int main(int argc, char **argv) {
    int width = -1, height = -1, generations = -1, mode = -1;
    struct timespec now, tmstart;
    clock_gettime(CLOCK_REALTIME, &tmstart);
    parseProgramOptions(argc, argv, &width, &height, &generations, &mode);
    runGame(width, height, generations, mode);
    clock_gettime(CLOCK_REALTIME, &now);
    double seconds = (double)((now.tv_sec+now.tv_nsec*1e-9) - (double)(tmstart.tv_sec+tmstart.tv_nsec*1e-9));
    printf("time %fs\n", seconds);
    return EXIT_SUCCESS;
}

/*******************
 * GAME  functions *
 *******************/

void runGame(int width, int height, int generations, int mode) {
    unsigned char ** gameMatrix = allocateGameSpace(width, height);
    unsigned char switchValuesFlag = 0;

    generateRandomGame(gameMatrix, width, height);
    for (int i = 0; i < generations; i++) {
        #if DEBUG
        printf("generation %d\n", i);
        printGameMatrix(gameMatrix, width, height, switchValuesFlag);
        #endif
        switch (mode) {
            case 0: 
                evolve_normal(gameMatrix, width, height, switchValuesFlag);
                break;
            case 1:
                evolve_omp(gameMatrix, width, height, switchValuesFlag);
                break;
            default:
                break;
        }
        
        switchValuesFlag = !switchValuesFlag;
        #if DEBUG
        usleep(200000);
        #endif
    }

}

void evolve_normal(unsigned char ** gameMatrix, int width, int height, unsigned char switchValuesFlag) {
    int nw, no, ne, w, c, e, sw, s, se;
        
    for_x {
        for_y {
            nw = (x == 0 || y == 0) ? 0 : (gameMatrix[x-1][y-1] & (switchValuesFlag+1)) > 0;
            no = (x == 0) ? 0 : (gameMatrix[x-1][y] & (switchValuesFlag+1)) > 0;
            ne = (x == 0 || y == height-1) ? 0 : (gameMatrix[x-1][y+1] & (switchValuesFlag+1)) > 0;
            w = (y == 0) ? 0 : (gameMatrix[x][y-1] & (switchValuesFlag+1)) > 0;
            c = (gameMatrix[x][y] & (switchValuesFlag+1)) > 0;
            e = (y == height-1) ? 0 : (gameMatrix[x][y+1] & (switchValuesFlag+1)) > 0;
            sw = (x == width-1 || y == 0) ? 0 : (gameMatrix[x+1][y-1] & (switchValuesFlag+1)) > 0;
            s = (x == width-1) ? 0 : (gameMatrix[x+1][y] & (switchValuesFlag+1)) > 0;
            se = (x == width-1 || y == height-1) ? 0 : (gameMatrix[x+1][y+1] & (switchValuesFlag+1)) > 0;

            gameMatrix[x][y] = setValuesCode(gameMatrix[x][y], lifeFunction(nw, no, ne, w, c, e, sw, s, se), switchValuesFlag);
        }
    }
}

void evolve_omp(unsigned char ** gameMatrix, int width, int height, unsigned char switchValuesFlag) {
    int nw, no, ne, w, c, e, sw, s, se;
        
    #pragma omp parallel for
    for_x {
        for_y {
            nw = (x == 0 || y == 0) ? 0 : (gameMatrix[x-1][y-1] & (switchValuesFlag+1)) > 0;
            no = (x == 0) ? 0 : (gameMatrix[x-1][y] & (switchValuesFlag+1)) > 0;
            ne = (x == 0 || y == height-1) ? 0 : (gameMatrix[x-1][y+1] & (switchValuesFlag+1)) > 0;
            w = (y == 0) ? 0 : (gameMatrix[x][y-1] & (switchValuesFlag+1)) > 0;
            c = (gameMatrix[x][y] & (switchValuesFlag+1)) > 0;
            e = (y == height-1) ? 0 : (gameMatrix[x][y+1] & (switchValuesFlag+1)) > 0;
            sw = (x == width-1 || y == 0) ? 0 : (gameMatrix[x+1][y-1] & (switchValuesFlag+1)) > 0;
            s = (x == width-1) ? 0 : (gameMatrix[x+1][y] & (switchValuesFlag+1)) > 0;
            se = (x == width-1 || y == height-1) ? 0 : (gameMatrix[x+1][y+1] & (switchValuesFlag+1)) > 0;

            gameMatrix[x][y] = setValuesCode(gameMatrix[x][y], lifeFunction(nw, no, ne, w, c, e, sw, s, se), switchValuesFlag);
        }
    }
}

/**
 * here some detailed comments :)
 *
 * . nw . n . ne
 * .  w . c . e
 * . sw . s . se
 */
unsigned char lifeFunction(int nw, int n, int ne, int w, int c, int e, int sw, int s, int se) {
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

unsigned char setValuesCode(unsigned char currentCode, int alive, unsigned char switchValuesFlag) {
    if (switchValuesFlag) {
        if (alive) return (currentCode | 1); 
        return (currentCode & 2); 
    } else {
        if (alive) return (currentCode | 2);
        return (currentCode & 1);
    }
}

/*******************
 * OTHER functions *
 *******************/

void parseProgramOptions(int argc, char **argv, int * width, int * height, int * generations, int * mode) {
    int option = 0;

    while ((option = getopt(argc, argv, "w:h:g:m:")) != -1) {
        switch (option) {
            case 'w': *width = atoi(optarg); break;
            case 'h': *height = atoi(optarg); break;
            case 'g': *generations = atoi(optarg); break;
            case 'm': *mode = atoi(optarg); break;
            default: printUsage();
                exit(EXIT_FAILURE);
        }
    }

    if (*width == -1 || *height == -1 || *generations == -1 || *mode == -1) {
        printUsage();
        exit(EXIT_FAILURE);
    }

    if (*mode < 0 || *mode > 1) {
        printUsage();
        exit(EXIT_FAILURE);
    }
}

void printUsage() {
    printf("Usage: gameoflife -w num -h num -g num -m num\n");
    printf("-w ... matrix width\n");
    printf("-h ... matrix height\n");
    printf("-g ... generations\n");
    printf("-m ... mode (0=normal, 1=openmp, 2=...)\n");
}

unsigned char ** allocateGameSpace(int width, int height) {
    unsigned char ** gameMatrix =(unsigned char **) malloc(width * sizeof(unsigned char *));
    for_x {
        gameMatrix[x] = (unsigned char *) malloc(height * sizeof(unsigned char));
    }
    return gameMatrix;
}

void generateRandomGame(unsigned char ** gameMatrix, int width, int height) {
    //srand(time(NULL));
    for_xy {
        gameMatrix[x][y] = rand() % 2;
    }
}

void printGameMatrix(unsigned char ** gameMatrix, int width, int height, unsigned char switchValuesFlag) {
    printf("\033[H");
    for_x {
        for_y {
            if ((gameMatrix[x][y] & (switchValuesFlag+1)) > 0) {
                printf("X ");
            } else {
                printf("  ");
            }
        }
        printf("\033[E");
    }
}