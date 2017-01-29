/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <omp.h>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>

/* game function prototypes */
void computeGameMatrix(unsigned char ** gameMatrix, int width, int height, int x, int y, unsigned char switchValuesFlag);
unsigned char lifeFunction(int nw, int n, int ne, int w, int c, int e, int sw, int s, int se);
unsigned char setValuesCode(unsigned char currentCode, int alive, unsigned char switchValuesFlag);
void checkResult(unsigned char ** gameMatrix, int width, int height, int generations);

/* game prototypes */
double runGame(int width, int height, int generations);
double runGameOMP(int width, int height, int generations, int numThreads, int verification);
double runGameCilk(int width, int height, int generations, int numThreads, int verification);

/* game evolve prototypes */
void evolve_normal(unsigned char ** gameMatrix, int width, int height, unsigned char switchValuesFlag);
void evolve_parallel(unsigned char ** gameMatrix, int width, int height, unsigned char switchValuesFlag, int threadId, int numThreads);

/* other prototypes */
void printUsage();
void generateRandomGame(unsigned char ** gameMatrix, int width, int height);
void printGameMatrix(unsigned char ** gameMatrix, int width, int height, unsigned char switchValuesFlag);
unsigned char ** allocateGameSpace(int width, int height);
void parseProgramOptions(int argc, char **argv, int * width, int * height, int * generations, int * mode, int * threads, int * verification);
double cilkTime();
void showDebugInfo(int generation, unsigned char ** gameMatrix, int width, int height, unsigned char switchValuesFlag);

/* defines */
#define for_x for (int x = 0; x < width; x++)
#define for_y for (int y = 0; y < height; y++)
#define for_xy for_x for_y

/* main */
int main(int argc, char **argv) {
    int width = -1, height = -1, generations = -1, mode = -1, threads = -1, verification = -1;
    parseProgramOptions(argc, argv, &width, &height, &generations, &mode, &threads, &verification);

    double time = 0;
    switch (mode) {
        case 0:
        time = runGame(width, height, generations);
        break;
        case 1:
        time = runGameOMP(width, height, generations, threads, verification);
        break;
        case 2:
        time = runGameCilk(width, height, generations, threads, verification);
        default: 
        break;
    }

    printf("time %fs\n", time);

    return EXIT_SUCCESS;
}

/*******************
 * GAME  functions *
 *******************/

double runGame(int width, int height, int generations) {
    unsigned char ** gameMatrix = allocateGameSpace(width, height);
    unsigned char switchValuesFlag = 0;
    generateRandomGame(gameMatrix, width, height);

    struct timespec now, tmstart;
    clock_gettime(CLOCK_REALTIME, &tmstart);
    for (int i = 0; i < generations; i++) {
        evolve_normal(gameMatrix, width, height, switchValuesFlag);
        showDebugInfo(i, gameMatrix, width, height, switchValuesFlag);
        switchValuesFlag = !switchValuesFlag;
    }
    clock_gettime(CLOCK_REALTIME, &now);
    printf("prevent super optimized O3 magic haha %d\n", gameMatrix[0][0]);

    return (double)((now.tv_sec+now.tv_nsec*1e-9) - (double)(tmstart.tv_sec+tmstart.tv_nsec*1e-9));
}

double runGameOMP(int width, int height, int generations, int numThreads, int verification) {
    unsigned char ** gameMatrix = allocateGameSpace(width, height);
    unsigned char switchValuesFlag = 0;
    generateRandomGame(gameMatrix, width, height);

    double seconds = omp_get_wtime();
    #pragma omp parallel num_threads(numThreads) 
    {
        for (int i = 0; i < generations; i++) {

            evolve_parallel(gameMatrix, width, height, switchValuesFlag, omp_get_thread_num(), numThreads);
            #pragma omp barrier

            #pragma omp single
            {
                showDebugInfo(i, gameMatrix, width, height, switchValuesFlag);
                switchValuesFlag = !switchValuesFlag;
            }
            #pragma omp barrier
        }
    }

    if (verification == 1) {
        checkResult(gameMatrix, width, height, generations);
    }

    return omp_get_wtime() - seconds;
}

double runGameCilk(int width, int height, int generations, int numThreads, int verification) {
    unsigned char ** gameMatrix = allocateGameSpace(width, height);
    unsigned char switchValuesFlag = 0;
    generateRandomGame(gameMatrix, width, height);

    double seconds = cilkTime();
    for (int i = 0; i < generations; i++) {
        cilk_for(int t = 0; t < numThreads; t++) {
            evolve_parallel(gameMatrix, width, height, switchValuesFlag, t, numThreads);
        }
        showDebugInfo(i, gameMatrix, width, height, switchValuesFlag);
        switchValuesFlag = !switchValuesFlag;
    }

    if (verification == 1) {
        checkResult(gameMatrix, width, height, generations);
    }

    return cilkTime() - seconds;
}

void checkResult(unsigned char ** gameMatrix, int width, int height, int generations) {
    unsigned char ** verificationMatrix = allocateGameSpace(width, height);
    unsigned char switchValuesFlag = 0;
    generateRandomGame(verificationMatrix, width, height);

    // evolve verfication matrix
    for (int i = 0; i < generations; i++) {
        evolve_normal(verificationMatrix, width, height, switchValuesFlag);
        switchValuesFlag = !switchValuesFlag;
    }

    unsigned char flag = 1;
    // compare gameMatrix to verificationMatrix
    for_xy {
        if (gameMatrix[x][y] != verificationMatrix[x][y]) {
            flag = 0;
        }
    }

    if (flag) {
        printf("-- result correct\n");
    } else {
        printf("-- result INcorrect\n");
    }
}

void evolve_normal(unsigned char ** gameMatrix, int width, int height, unsigned char switchValuesFlag) {
    for_xy {
        computeGameMatrix(gameMatrix, width, height, x, y, switchValuesFlag);
    }
}

void evolve_parallel(unsigned char ** gameMatrix, int width, int height, unsigned char switchValuesFlag, int threadId, int numThreads) {
    int rowBlock = height / numThreads;
    int rowStart = threadId * rowBlock;
    int rowEnd = rowStart + rowBlock;

    for_x {
        for (int y = rowStart; y < rowEnd; y++) {
            computeGameMatrix(gameMatrix, width, height, x, y, switchValuesFlag);
        }
    }
}

void computeGameMatrix(unsigned char ** gameMatrix, int width, int height, int x, int y, unsigned char switchValuesFlag) {
    int nw, no, ne, w, c, e, sw, s, se;

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

/**
 * . nw . n . ne
 * .  w . c . e
 * . sw . s . se
 */
unsigned char lifeFunction(int nw, int n, int ne, int w, int c, int e, int sw, int s, int se) {
    int living = nw + n + ne + w + e + sw + s + se;
    if (c == 0) {
        if (living == 3) return 1;
        return 0;
    } else {
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

void parseProgramOptions(int argc, char **argv, int * width, int * height, int * generations, int * mode, int * threads, int * verification) {
    int maxThreads = 0;
    int option = 0;

    while ((option = getopt(argc, argv, "w:h:g:m:t:v")) != -1) {
        switch (option) {
            case 'w': *width = atoi(optarg); break;
            case 'h': *height = atoi(optarg); break;
            case 'g': *generations = atoi(optarg); break;
            case 'm': *mode = atoi(optarg); break;
            case 't': *threads = atoi(optarg); break;
            case 'v': *verification = 1; break;
            default: printUsage();
            exit(EXIT_FAILURE);
        }
    }

    if (*width == -1 || *height == -1 || *generations == -1 || *mode == -1) {
        printUsage();
    }

    if (*width < 0 || *height < 0 || *generations < 0) {
        fprintf(stderr, "err: use positive numbers for width, height and generations\n");
        printUsage();
    }

    if (*mode < 0 || *mode > 2) {
        printUsage();
    }

    if (*mode == 1 || *mode == 2) {
        switch (*mode) {
            case 1:
            maxThreads = omp_get_max_threads();
            break;
            case 2:
            maxThreads = __cilkrts_get_nworkers();
            break;
            default:
            break;
        }

        if (*threads == -1) {
            printf("no -t option, using max number of threads\n");
            printf("max number of available threads: %d\n", maxThreads);
            *threads = maxThreads;
        } else if (*threads > maxThreads) {
            fprintf(stderr, "err: you cannot use more than max number of threads\n");
            printUsage();
        } else if (*threads < 0) {
            fprintf(stderr, "err: use positive number of threads\n");
            printUsage(); 
        }

        if ((*height % *threads) != 0) {
            fprintf(stderr, "height is not dividable by number of threads\n");
            printUsage();
        }
    }
}

void printUsage() {
    printf("Usage: gameoflife -w num -h num -g num -m num -t num -v\n");
    printf("-w ... matrix width\n");
    printf("-h ... matrix height\n");
    printf("-g ... generations\n");
    printf("-m ... mode (0=normal, 1=openmp, 2=cilk)\n");
    printf("-t ... number of threads to be used\n");
    printf("-v ... enable verification with seq solution\n");
    printf("NOTE: when using openmp/cilk then height must be dividable through the number of threads\n");
    exit(EXIT_FAILURE);
}

unsigned char ** allocateGameSpace(int width, int height) {
    unsigned char ** gameMatrix =(unsigned char **) malloc(width * sizeof(unsigned char *));
    for_x {
        gameMatrix[x] = (unsigned char *) malloc(height * sizeof(unsigned char));
    }
    return gameMatrix;
}

void generateRandomGame(unsigned char ** gameMatrix, int width, int height) {
    srand(100);
    for_xy {
        gameMatrix[x][y] = rand() % 2;
    }
}

void printGameMatrix(unsigned char ** gameMatrix, int width, int height, unsigned char switchValuesFlag) {
    printf("\033[H");
    for_x {
        for_y {
            if ((gameMatrix[x][y] & (switchValuesFlag+1)) > 0) {
                printf("\033[07m  \033[m");
            } else {
                printf("  ");
            }
        }
        printf("\n");
    }
}

double cilkTime() {
    struct timeval now; gettimeofday(&now,NULL);
    return (double)(((long double)now.tv_usec + (long double)now.tv_sec*1000000) / 1000000);
}

void showDebugInfo(int generation, unsigned char ** gameMatrix, int width, int height, unsigned char switchValuesFlag) {
    #if DEBUG
    printf("generation %d\n", generation);
    printGameMatrix(gameMatrix, width, height, switchValuesFlag);
    usleep(200000);
    #endif
}