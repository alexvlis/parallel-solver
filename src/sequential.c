#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <sys/time.h>

#define MAX_SIZE 5000
#define ERROR2(msg, x) (printf("ERROR: %s%d.\n", msg, x))
#define ERROR(msg) (printf("ERROR: %s.\n", msg))
#define SWAP(x, y)       \
        ({               \
            float **tmp; \
            tmp = x;     \
            x = y;       \
            y = tmp;     \
        })

static int loadfile(char *filename, float **array, unsigned int size);
static inline void print_array(float **array, const unsigned int size);
static float **create_array(const unsigned int size);
static void destroy_array(float **array, const unsigned int size);

int main (int argc, char **argv) {
    unsigned int size;
    float precision;
    float value;
    struct timeval tStart, tEnd;
    double elapsedTime;
    bool printFlag;
    float **array1;
    float **array2;
    float **read_array;
    float **write_array;
    bool done = false;
    int counter = 0;

    /* Start measuring time */
    gettimeofday(&tStart, NULL);

    if (argc < 4) {
        ERROR("Minimum number of arguments is 3");
        exit(EXIT_FAILURE);
    }
    size = atoi(argv[1]);
    precision = atof(argv[2]);
    printFlag = atoi(argv[3]);
    if (size > MAX_SIZE) {
        ERROR2("The maximum array size is ", MAX_SIZE);
        exit(EXIT_FAILURE);
    }
    array1 = create_array(size);
    array2 = create_array(size);

    if (loadfile("array.txt", array1, size)) {
        ERROR("Failed to load file");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            array2[i][j] = array1[i][j];
        }
    }
    size -= 2;

    read_array = array1;
    write_array = array2;
    int count = 0;
    while(!done) {
        for (int i = 1; i < size+1; i++) {
            for (int j = 1; j < size+1; j++) {
                value = (read_array[i][j+1] +
                         read_array[i][j-1] +
                         read_array[i-1][j] +
                         read_array[i+1][j])/4;
                if (fabs(read_array[i][j] - value) < precision) {
                    counter++;
                    if (counter == (size*size)) {
                        done = true;
                        write_array[i][j] = value;
                        break;
                    }
                } 
                write_array[i][j] = value;
            }
        }
        count++;
        SWAP(read_array, write_array);
        counter = 0;
    }
    printf("Calculated %d times.\n", count);
    if (printFlag) {
        print_array(read_array, size);
        print_array(write_array, size);
    }
    destroy_array(array1, size);    
    destroy_array(array2, size);

    /* Stop measuring time */
    gettimeofday(&tEnd, NULL);
    elapsedTime = (tEnd.tv_sec - tStart.tv_sec) * 1000.0;      /* sec to ms */
    elapsedTime += (tEnd.tv_usec - tStart.tv_usec) / 1000.0;   /* us to ms */
    printf("Time elapsed: %lf ms.\n", elapsedTime);

    return 0;
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

static inline void print_array(float **array, const unsigned int size) {
    printf("The array is:\n");
    for (int i = 0; i < size+2; i++) {
        for (int j = 0; j < size+2; j++) {
            printf("%f ", array[i][j]);
        }
        printf("\n");
    }
}

static int loadfile(char *filename, float **array, unsigned int size) {
    FILE *file;

    file = fopen(filename, "r");
    if (NULL == file) {
        ERROR("Could not open file.");
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
