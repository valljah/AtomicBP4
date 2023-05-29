#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

const int MAX_CNT = 100*1000*1000;

int64_t balance;
sem_t global_sem;

void *change_balance(void *args) {
    int64_t *amt = (int64_t *) args;

    // Blockieren des Threads, falls anderer Thread derzeit im kritischen Bereich
    sem_wait(&global_sem);
    balance = balance + *amt;
    int64_t new_balance = balance;
    // Freigeben der Blockade nach Verlassen des kritischen Bereichs
    sem_post(&global_sem);

    return (void *) new_balance;
}

void *change_balance_atomic(void *args) {

}

int main(int argc, char ** argv) {
    // Dokumentation siehe main_atomic.c

    unsigned int n_threads = 4, repetitions = MAX_CNT;
    pthread_t *pthread_array;
    int64_t *random_values, random_sign, original_balance = 2000, difference = 0;
    int64_t **results;

    if (argc >= 2)
        n_threads = strtoul(argv[1], NULL, 10);
    if (argc >= 3)
        original_balance = strtol(argv[2], NULL, 10);
    if (argc >= 4)
        repetitions = strtoul(argv[3], NULL, 10);

    balance = original_balance;
    printf("Input: n_threads: %u, balance: %lld, repetitions: %u\n", n_threads, balance, repetitions);

    pthread_array = calloc(n_threads, sizeof(pthread_t));
    random_values = calloc(n_threads, sizeof(int64_t));
    results = calloc(n_threads, sizeof(int64_t *));

    // Intialisieren der globalen Semaphore mit Wert 1
    sem_init(&global_sem, 0, 1);

    srand(time(NULL));
    for (int i = 0; i < repetitions; i += (int) n_threads) {
        for (size_t si = 0; si < n_threads; si++) {
            random_sign = rand() % 1000;
            random_sign = (random_sign > 500) ? 1 : -1;
            random_values[si] = random_sign * (rand() % 500);
            difference += random_values[si];
            pthread_create(&pthread_array[si], NULL, &change_balance, &random_values[si]);
        }

        for (size_t si = 0; si < n_threads; si++) {
            pthread_join(pthread_array[si], (void**) &results[si]);
        }
    }

    printf("Original balance: \t%+lld\n"
           "New balance: \t\t%+lld\n"
           "Measured difference: \t%+lld\n"
           "Actual difference: \t%+lld\n",
           original_balance, balance, difference, balance - original_balance);

    free(pthread_array);
    free(random_values);
    free(results);

    // Zerstören der Semaphore
    sem_destroy(&global_sem);
}
