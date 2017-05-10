#include <sys/time.h>
#include <stdbool.h>

#include "solver.h"

/* Definition of macros */
#define MAX_SIZE 5000
#define ERROR2(msg, x) (printf("ERROR: %s%d.\n", msg, x))
#define ERROR(msg) (printf("ERROR: %s.\n", msg))


/* Function Protoypes */
static int loadfile(const char *filename, float **array, const unsigned int size);
static void init_points(point_t *points, const unsigned short threads);
static inline void print_array(float **array, const unsigned int size);
static float **create_array(const unsigned int size);
static void destroy_array(float **array, const unsigned int size);
static inline void assign_work(const unsigned int i, point_t *start, point_t *end, 
                                const unsigned int step);
static bool allocate_work(point_t *start, point_t *end, 
                            const unsigned short threadN, 
                            const unsigned int size);

int main(int argc, char** argv) {
    int threadN;
    float precision;
    int rc;
    bool printFlag;
    unsigned int size;
    float **array1;
    float **array2;
    int i;
    struct timeval tStart, tEnd;
    double elapsedTime;
    threadinfo_t *info;
    pthread_t *threads;
    point_t *start;
    point_t *end;
    pthread_barrier_t barrier;
    shared_counter_t finished;

    /* Start measuring time */
    gettimeofday(&tStart, NULL);

    /* Do argument check */
    if (argc < 5) {
        ERROR("Minimum number of arguments is 4");
        printf("Usage: ./solver <size> <threads> <precision> <print flag>\n");
        exit(EXIT_FAILURE);
    }
    size = atoi(argv[1]);
    threadN = atoi(argv[2]);
    precision = atof(argv[3]);
    printFlag = atoi(argv[4]);

    /* Check that arguments are valid */
    if (threadN >= size -2) {
        ERROR("The number of threads cannot be larger than the dimensions of the array");
        exit(EXIT_FAILURE);
    }
   else if (threadN <= 1) {
        ERROR("The number of threads must be greater than 1");
        exit(EXIT_FAILURE);
    }
    if (size > MAX_SIZE) {
        ERROR2("The maximum array size is ", MAX_SIZE);
        exit(EXIT_FAILURE);
    }
    if (precision <= 0) {
        ERROR("The precision must be a positive non-zero value");
        exit(EXIT_FAILURE);
    }
    
    /* We're good hence create arrays */
    array1 = create_array(size);
    array2 = create_array(size);

    /* Read the file that contains the matrix */
    if (loadfile("array.txt", array1, size)) {
        ERROR("Failed to load file");
        exit(EXIT_FAILURE);
    }
    /* Copy values in both arrays */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            array2[i][j] = array1[i][j];
        }
    }
    size -= 2;

    /* Create structures that will tell the threads which region to do */
    start = malloc(threadN * sizeof(point_t));
    end = malloc(threadN * sizeof(point_t));
    init_points(start, threadN);
    init_points(end, threadN);

    /* Divide the work for each worker */
    if (!allocate_work(start, end, threadN, size)) {
        ERROR("Could not allocate work");
        goto exit;
    }

    /* Initialize thread structures */
    threads = malloc(threadN * sizeof(pthread_t));
    info = malloc(threadN * sizeof(threadinfo_t));
    pthread_barrier_init(&barrier, NULL, threadN);
    finished.count = threadN;
    pthread_mutex_init(&finished.lock, NULL);

    /* Generate threads */
    for (i = 0; i < threadN; i++) {
        /* Prepare thread information */
        info[i].threadN = threadN;
        info[i].start = &start[i];
        info[i].end = &end[i];
        info[i].precision = precision;
        info[i].array1 = array1;
        info[i].array2 = array2;
        info[i].barrier = &barrier;
        info[i].finished = &finished;

        /* Create thread */
        rc = pthread_create(&threads[i], NULL, solver, (void *)&info[i]);
        if (rc) {
            ERROR2("Failed to create thread. Code is ", rc);
            goto pthread_exit;
        }
    }
pthread_exit:
    /* Wait for everyone to finish */
    for (int i = 0; i < threadN; i++) {
        pthread_join(threads[i], NULL);
    }

    /* Print the array if specified by the user */
    if (printFlag) {
        print_array(array1, size);
        print_array(array2, size);
    }

    /* Clean up thread data structures */
    free(threads);
    free(info);
    pthread_barrier_destroy(&barrier);

exit:
    /* Clean up memory */
    free(start);
    free(end);
    destroy_array(array1, size);
    destroy_array(array2, size);

    /* Stop measuring time */
    gettimeofday(&tEnd, NULL);
    elapsedTime = (tEnd.tv_sec - tStart.tv_sec) * 1000.0;      /* sec to ms */
    elapsedTime += (tEnd.tv_usec - tStart.tv_usec) / 1000.0;   /* us to ms */
    printf("Time elapsed: %f ms.\n", elapsedTime);

    pthread_exit(NULL);
}

static inline void print_array(float **array, const unsigned int size) {
    printf("The array is:\n");
    for (int i = 0; i < size+2; i++) {
        for (int j = 0; j < size+2; j++) {
            printf("%f ", array[i][j]);
        }
        printf("\n");
    }
}

static float **create_array(const unsigned int size) {
    float **array;

    array = malloc(size * sizeof(float *));
    for (int i = 0; i < size; i++) {
        array[i] = malloc(size * sizeof(float));
    }
    return array;
}

static void destroy_array(float **array, const unsigned int size) {
    for (int i = 0; i < size+2; i++) {
        free(array[i]);
    }
    free(array);
}

static int loadfile(const char *filename, float **array, const unsigned int size) {
    FILE *file;

    file = fopen(filename, "r");
    if (NULL == file) {
        ERROR("Could not open file");
        return -1;
    }
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            fscanf(file, "%f", &array[i][j]);
        }
    }
    fclose(file);
    return 0;
}

static void init_points(point_t *points, const unsigned short threads) {
    for (int i = 0; i < threads; i++) {
        points[i].row = 1;
        points[i].col = 1;
    }
}

static bool allocate_work(point_t *start, point_t *end, 
                            const unsigned short threadN, 
                            const unsigned int size) {
    unsigned int step;

    /* Since we divide in slices all the end columns are the last ones */
    for (int i = 0; i < threadN; i++) {
        end[i].col = size + 1;
    }
    if ((size)%threadN == 0) {
        step = size/threadN;
        for (int i = 0; i < threadN; i++) {
            assign_work(i, start, end, step);
        }
    } else {
        unsigned int tmp_size = size;
        unsigned short threads = threadN;
        
        /* Decrement both until they are divisible */
        while(tmp_size%threads != 0) {
            tmp_size--;
            threads--;
        }
        step = tmp_size/threads;
        for (int i = 0; i < threads; i++) {
            assign_work(i, start, end, step);
        }
        /* Use different step for what is left */
        step = (size - tmp_size)/(threadN - threads);
        for (int i = threads; i < threadN; i++) {
            assign_work(i, start, end, step);
        }
    }
    return true;
}

static inline void assign_work(const unsigned int i, point_t *start, point_t *end, 
                                const unsigned int step) {
    if (i != 0) {
        start[i].row = start[i-1].row + step;
        end[i].row = end[i-1].row + step;
    } else {
        end[i].row += step;
    }
}
