#ifndef SOLVER_H
#define SOLVER_H

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <math.h>

typedef struct {
    unsigned short row;
    unsigned short col;
} point_t;

typedef struct {
    unsigned int count;
    pthread_mutex_t lock;
} shared_counter_t;

/* This is all the information a thread needs to do relaxation */
typedef struct {
    float precision;
    unsigned short threadN;
    float **array1;
    float **array2;
    point_t *start;
    point_t *end;
    shared_counter_t *finished;
    pthread_barrier_t *barrier;
} threadinfo_t;

void *solver(void *info);

#endif
