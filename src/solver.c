#include "solver.h"

#define SWAP(x, y)       \
        ({               \
            float **tmp; \
            tmp = x;     \
            x = y;       \
            y = tmp;     \
        })


void *solver(void *info) {
    int counter;
    float value;
    float **read_array;
    float **write_array;
    shared_counter_t *finished;    

    threadinfo_t *data = (threadinfo_t *) info;
    
    /* Calculate the number of elements we have to calculate */
    unsigned int cells = ((data->end->col) - data->start->col) * 
                            ((data->end->row) - data->start->row);

    /* Pass a few parameters to local variables for code clarity */ 
    finished = data->finished;
    read_array = data->array1;
    write_array = data->array2;

    /* Start consumer loop */
    counter = 0;
    for(;;) {
        for (int i = data->start->row; i < data->end->row; i++) {
            for (int j = data->start->col; j < data->end->col; j++) {
                value = (read_array[i+1][j] + 
                         read_array[i][j+1] + 
                         read_array[i][j-1] + 
                         read_array[i-1][j])/4;
                if (fabs(read_array[i][j] - value) < data->precision) {
                    /* We have reached required precision on this element */
                    counter++; 

                    /* Check if all the elements are at required precision */
                    if (counter == cells) {
                        pthread_mutex_lock(&finished->lock);
                        finished->count--;
                        pthread_mutex_unlock(&finished->lock);
                    }
                }
                write_array[i][j] = value;
            }
        }
        counter = 0; /* We must reset the counter in every iteration */
        SWAP(read_array, write_array); /* Swap the arrays around */
    
        /* We are done, wait for the last one */
        pthread_barrier_wait(data->barrier);

        if (finished->count == 0) {
            /* If everyone has finished we must exit */
            pthread_exit(NULL);
        } else {
            if (finished->count != data->threadN) {
                pthread_mutex_lock(&finished->lock);
                finished->count = data->threadN;
                pthread_mutex_unlock(&finished->lock);
            }
        }
        pthread_barrier_wait(data->barrier);
    }
}
