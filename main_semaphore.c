#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <inttypes.h>
#include <pthread.h>
#include <time.h>

const int MAX_CNT = 100 * 1000 * 1000;
int calculations_per_thread;

// Globale Semaphore
sem_t global_sem;
int64_t balance;

void *change_balance_atomic(void *args) {
    int64_t random_sign, random_value;
    int64_t *difference_per_thread = malloc(sizeof(int64_t));
    *difference_per_thread = 0;

    // Lokales Abspeichern von calculations_per_thread
    int local_calculations_per_thread = calculations_per_thread;
    for (int i = 0; i < local_calculations_per_thread; i++) {
        // Generieren eines zufälligen Wertes mit zufälligem Vorzeichen zwischen -100 und 100
        random_sign = rand() % 10000;
        random_sign = (random_sign >= 5000) ? 1 : -1;
        random_value = random_sign * (rand() % 100);

        // Messen der tatsächlichen Differenz
        *difference_per_thread += random_value;
        sem_wait(&global_sem);
        balance += random_value;
        sem_post(&global_sem);
    }

    return (void *) difference_per_thread;
}

int main(int argc, char **argv) {
    // Defaultwert der Anzahl Threads = 4, Defaultwert der insg. Berechnungen = MAX_CNT
    unsigned int n_threads = 4, repetitions = MAX_CNT;

    pthread_t *pthread_array; // Array der benötigten Threads

    int64_t *random_values, **results, random_sign, original_balance = 100000, difference = 0;
    // random_values: Array für Übergabe zufälliger Werte an die jeweiligen Threads
    // results: Array zum Abspeichern der Ergebnisse
    // random_sign: Variable für Bestimmung eines zufälligen Vorzeichens
    // original_balance: Defaultwert auf 100000
    // difference: Variable zur Messung der tatsächlich gemessenen Veränderung von balance

    // Keine Fehlerbehandlung bei falscher Eingabe. Es wird erwartet, dass der Benutzer die Kommandozeile
    // korrekt verwendet. Bei Weglassen der Argumente werden die Defaultwerte verwendet.
    // ./main_atomic [Anzahl Threads] [Initialwert Kontostand] [Anzahl Berechnungen]
    if (argc >= 2)
        n_threads = strtoul(argv[1], NULL, 10);
    if (argc >= 3)
        original_balance = strtol(argv[2], NULL, 10);
    if (argc >= 4)
        repetitions = strtoul(argv[3], NULL, 10);

    // Wenn explizit angegeben, wird original_balance in balance hineingeschrieben
    balance = original_balance;
    // Jeder Thread führt jeweils repetitions / n_threads Rechenschritte durch
    calculations_per_thread = repetitions / n_threads;
    printf("Input: n_threads: %u, balance: %lld, repetitions: %u\n", n_threads, balance, repetitions);

    // Allozieren der Arrays
    pthread_array = calloc(n_threads, sizeof(pthread_t));
    random_values = calloc(n_threads, sizeof(int64_t));
    results = calloc(n_threads, sizeof(int64_t *));

    // Seed für den RNG basierend auf Systemzeit
    srand(time(NULL));

    // Initialisieren der globalen Semaphore mit Wert 1
    sem_init(&global_sem, 0, 1);

    for (size_t si = 0; si < n_threads; si++)
        pthread_create(&pthread_array[si], NULL, &change_balance_atomic, NULL);

    for (size_t si = 0; si < n_threads; si++) {
        pthread_join(pthread_array[si], (void **) &results[si]);
        // Übernehmen der tatsächlich gemessenen Differenzen
        difference += *results[si];
        free(results[si]);
    }

    printf("Ursprünglicher Kontostand: \t %lld\n"
           "Neuer Kontostand: \t\t %lld\n"
           "Gemessene Differenz: \t\t%+lld\n"
           "Tatsächliche Differenz: \t%+lld\n",
           original_balance, balance, difference, balance - original_balance);

    // Befreien der Arrays
    free(pthread_array);
    free(random_values);
    free(results);

    // Zerstören der globalen Semaphore
    sem_destroy(&global_sem);
}
