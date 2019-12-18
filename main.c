//
// Created by christian on 06/09/2019.
//

#pragma GCC optimize("O3", "unroll-loops", "omit-frame-pointer", "inline") //Optimization flags
#pragma GCC option("arch=native", "tune=native", "no-zero-upper") //Enable AVX
#pragma GCC target("avx")  //Enable AVX

#include <time.h>    // for clock_t, clock(), CLOCKS_PER_SEC
#include <sys/time.h>
#include <stdio.h> //AVX/SSE Extensions are included in stdio.h
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>


//define matrix size (in this case we'll use a square matrix)
#define DIM 1000 //DO NOT EXCEED 10000 (modification to the stack size needed)

float matrix[DIM][DIM];
float result_matrix[DIM][DIM];

float *matrix_ptr = (float *) &matrix;
float *result_ptr = (float *) &result_matrix;

// set the number of logical cores to 1 (just in case the auto-detection doesn't work properly)
int cores = 1;

//functions prototypes
void single_multiply(int rowStart, int rowEnd);

void *thread_multiply(void *offset);

int detect_number_of_cores();

void fill_matrix();

int old_monothread_algorithm();

int main() {
    printf("");

    //two instructions needed for pseudo-random float numbers
    srand((unsigned int) time(NULL));

    //detect the number of active cores
    cores = detect_number_of_cores();

    //matrix filling with random float values
    fill_matrix();

    printf("------------- MATRIX MULTIPLICATION -------------\n");
    printf("--- multi-thread (vectorization enabled) v1.0 ---\n");

//    printf("\n ORIGINAL MATRIX");
//    for(int c=0; c<DIM; c++){
//        printf("\n");
//        for(int k=0; k<DIM; k++){
//            printf("%f \t", matrix[c][k]);
//        }
//    }

    //uncomment and modify this value to force a particular number of threads (not recommended)
    //cores = 6;

    printf("\n Currently using %i cores", cores);

    printf("\n Matrix size:  %i x %i", DIM, DIM);

    //time detection struct declaration
    struct timeval start, end;
    gettimeofday(&start, NULL);

    //decisional tree for the number of threads to be used
    if (cores == 0 || cores == 1 || cores > DIM) {
        //passing 0 because it has to start from the first row
        single_multiply(0, DIM);

        gettimeofday(&end, NULL);

        long seconds = (end.tv_sec - start.tv_sec);
        long micros = ((seconds * 1000000) + end.tv_usec) - (start.tv_usec);

        printf("\n\n Time elapsed is %ld seconds and %ld micros\n", seconds, micros);
        return 0;

    } else {

        //split the matrix in more parts (as much as the number of active cores)
        int rows_por_thread = DIM / cores;
        printf("\n Rows por Thread: %i", rows_por_thread);
        //calculate the rest of the division (if there is one obviously)
        int rest = DIM % cores;
        printf("\n Rest: %i \n", rest);

        if (rest == 0) {
            //execute just the multi-thread function n times
            int times = rows_por_thread;

            //create an array of thread-like objects
            pthread_t threads[cores];
            //create an array with the arguments for each thread
            int thread_args[cores];

            //launching the threads according to the available cores
            int i = 0;
            int error;
            for (int c = 0; c < DIM; c += rows_por_thread) {
                thread_args[i] = c;
                i++;
            }
            for (int c = 0; c < cores; c++) {
                error = pthread_create(&threads[c], NULL, thread_multiply, (void *) &thread_args[c]);
                if (error != 0) {
                    printf("\n Error in thread %i creation", c);
                }
                //printf("created thread n %i with argument: %i \n", c, thread_args[c]);
            }
            printf("\n ... working ...");
            for (int c = 0; c < cores; c++) {
                error = pthread_join(threads[c], NULL);
                if (error != 0) {
                    printf("\n Error in thread %i join", c);
                }
                //printf("\n Waiting to join thread n: %i", c);
            }

        } else {

            //THE PROBLEM MUST BE INSIDE THIS ELSE STATEMENT

            //execute the multi-thread function n times and the single function th rest remaining times
            printf("\n The number of cores is NOT a divisor of the size of the matrix. \n");

            //create an array of thread-like objects
            pthread_t threads[cores];
            //create an array with the arguments for each thread
            int thread_args[cores];

            //launching the threads according to the available cores
            int i = 0;  //counter for the thread ID
            int entrypoint_residual_rows = 0;   //first unprocessed residual row

            //launching the threads according to the available coreS
            for (int c = 0; c < DIM - rest; c += rows_por_thread) {
                thread_args[i] = c;
                i++;
            }

            entrypoint_residual_rows = cores * rows_por_thread;
            int error;
            //launch the threads
            for (int c = 0; c < cores; c++) {
                error = pthread_create(&threads[c], NULL, thread_multiply, (void *) &thread_args[c]);
                if (error != 0) {
                    printf("\n Error in thread %i creation, exiting...", c);
                }
                //printf("created thread n %i with argument: %i \n", c, thread_args[c]);
            }

            printf("\n ... working ...\n");
            //join all the previous generated threads
            for (int c = 0; c < cores; c++) {
                pthread_join(threads[c], NULL);
                //printf("\n Waiting to join thread n: %i", c);
            }
            printf("\n entry-point index for the single function %i ", entrypoint_residual_rows);
            single_multiply(entrypoint_residual_rows, DIM);
        }
    }

//    printf("\n MULTIPLIED MATRIX");
//    for (int c = 0; c < DIM; c++) {
//        printf("\n");
//        for (int k = 0; k < DIM; k++) {
//            printf("%f \t", result_matrix[c][k]);
//        }
//    }

    gettimeofday(&end, NULL);

    printf("\n All threads joined correctly");

    long seconds = (end.tv_sec - start.tv_sec);
    long micros = ((seconds * 1000000) + end.tv_usec) - (start.tv_usec);

    printf("\n\n Time elapsed is %ld seconds and %ld micros\n", seconds, micros);
    return 0;
}

//detect number of cores of the CPU (logical cores)
int detect_number_of_cores() {
    return (int) sysconf(_SC_NPROCESSORS_ONLN); // Get the number of logical CPUs.
}

//matrix filling function
void fill_matrix() {
    float a = 5.0;
    for (int c = 0; c < DIM; c++)
        for (int d = 0; d < DIM; d++) {
            matrix[c][d] = (float) rand() / (float) (RAND_MAX) * a;
        }
}

//row by row multiplication algorithm (mono-thread version)
void single_multiply(int rowStart, int rowEnd) {
    for (int i = rowStart; i < rowEnd; i++) {
        //printf("\n %i", i);
        for (int j = 0; j < DIM; j++) {
            *(result_ptr + i * DIM + j) = 0;
            for (int k = 0; k < DIM; k++) {
                *(result_ptr + i * DIM + j) += *(matrix_ptr + i * DIM + k) * *(matrix_ptr + k * DIM + j);
            }
        }
    }
}

//thread for the multiplication algorithm
void *thread_multiply(void *offset) {
    //de-reference the parameter passed by the main-thread
    int *row_offset = (int *) offset;
    //printf(" Starting at line %i ending at line %i \n ", *row_offset, *row_offset + (DIM / cores));
    single_multiply(*row_offset, *row_offset + (DIM / cores));
    //printf("\n ended at line %i", *row_offset + (DIM / cores));
    return NULL;
}
