#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <inttypes.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

const int MAX_CNT = 100 * 1000 * 1000;

atomic_llong balance = ATOMIC_VAR_INIT(2000);

void *change_balance_atomic(void *args) {
    int64_t *amt = (int64_t *) args;

    int64_t new_balance = atomic_fetch_add(&balance, *amt) + *amt;

    return (void *) new_balance;
}

int main(int argc, char ** argv) {
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

    atomic_store(&balance, original_balance);
    printf("Input: n_threads: %u, balance: %lld\n", n_threads, balance);

    pthread_array = calloc(n_threads, sizeof(pthread_t));
    random_values = calloc(n_threads, sizeof(int64_t));
    results = calloc(n_threads, sizeof(int64_t *));

    srand(time(NULL));
    for (int i = 0; i < repetitions; i += (int) n_threads) {
        for (size_t si = 0; si < n_threads; si++) {
            random_sign = rand() % 10;
            random_sign = (random_sign > 5) ? 1 : -1;
            random_values[si] = random_sign * (rand() % 100);
            difference += random_values[si];
            pthread_create(&pthread_array[si], NULL, &change_balance_atomic, &random_values[si]);
        }

        for (size_t si = 0; si < n_threads; si++) {
            pthread_join(pthread_array[si], (void**) &results[si]);
        }
    }

    const long long final_balance = atomic_load(&balance);

    printf("Original balance: \t%+lld\n"
           "New balance: \t\t%+lld\n"
           "Measured difference: \t%+lld\n"
           "Actual difference: \t%+lld\n",
           original_balance, final_balance, difference, final_balance - original_balance);

    free(pthread_array);
    free(random_values);
    free(results);
}
