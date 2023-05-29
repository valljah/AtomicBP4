#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <time.h>

const int MAX_CNT = 100*1000*1000;

double balance;
sem_t global_sem;

void *change_balance(void *args) {
    double *amt = (double *) args;
    double *new_balance = malloc(sizeof(double));

    sem_wait(&global_sem);
    balance = balance + *amt;
    *new_balance = balance;
    sem_post(&global_sem);

    return (void *) new_balance;
}

int main(int argc, char ** argv) {
    unsigned int n_threads = 4;
    pthread_t *pthread_array;
    double *random_values, random_sign, original_balance = 2000.0, difference = 0.0;
    double **results;

    if (argc >= 2)
        n_threads = strtol(argv[1], NULL, 10);

    if (argc == 3)
        original_balance = strtod(argv[2], NULL);

    balance = original_balance;
    printf("Input: n_threads: %u, balance: %f\n", n_threads, balance);

    pthread_array = calloc(n_threads, sizeof(pthread_t));
    random_values = calloc(n_threads, sizeof(double));
    results = calloc(n_threads, sizeof(double *));
    sem_init(&global_sem, 0, 1);

    srand(time(NULL));
    for (int i = 0; i < MAX_CNT; i += (int) n_threads) {
        for (size_t si = 0; si < n_threads; si++) {
            random_sign = rand() % 1000;
            random_sign = (random_sign > 500) ? 1.0 : -1.0;
            random_values[si] = random_sign * (rand() % 500);
            difference += random_values[si];
            pthread_create(&pthread_array[si], NULL, &change_balance, &random_values[si]);
        }

        for (size_t si = 0; si < n_threads; si++) {
            pthread_join(pthread_array[si], (void**) &results[si]);
            free(results[si]);
        }
    }

    printf("Original balance: %+10.3f\n"
           "New balance: %+10.3f\n"
           "Measured difference: %+10.3f\n"
           "Actual difference: %+10.3f\n",
           original_balance, balance, difference, balance - original_balance);

    free(pthread_array);
    free(random_values);
    free(results);
    sem_destroy(&global_sem);
}
