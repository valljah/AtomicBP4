#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <inttypes.h>
#include <pthread.h>
#include <time.h>

const int MAX_CNT = 100 * 1000 * 1000;

// Defaultwert von balance liegt bei 2000
atomic_llong balance = ATOMIC_VAR_INIT(2000);

void *change_balance_atomic(void *args) {
    int64_t *amt = (int64_t *) args;

    // Abspeichern des neuen Kontostandes in eigener Variable
    // balance wird um amt verändert
    int64_t new_balance = atomic_fetch_add(&balance, *amt) + *amt;

    return (void *) new_balance;
}

int main(int argc, char ** argv) {
    // Defaultwert der Anzahl Threads = 4, Defaultwert der insg. Berechnungen = MAX_CNT
    unsigned int n_threads = 4, repetitions = MAX_CNT;

    pthread_t *pthread_array; // Array der benötigten Threads

    int64_t *random_values, random_sign, original_balance = 2000, difference = 0;
    // random_values: Array für Übergabe zufälliger Werte an die jeweiligen Threads
    // random_sign: Variable für Bestimmung eines zufälligen Vorzeichens
    // original_balance: Defaultwert auf 2000
    // difference: Variable zur Messung der tatsächlich gemessenen Veränderung von balance

    int64_t **results; // Array für Ergebnisse der Threads bei pthread_join

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
    atomic_store(&balance, original_balance);
    printf("Input: n_threads: %u, balance: %lld, repetitions: %u\n", n_threads, balance, repetitions);

    // Allozieren der Arrays
    pthread_array = calloc(n_threads, sizeof(pthread_t));
    random_values = calloc(n_threads, sizeof(int64_t));
    results = calloc(n_threads, sizeof(int64_t *));

    srand(time(NULL));
    for (int i = 0; i < repetitions; i += (int) n_threads) {
        for (size_t si = 0; si < n_threads; si++) {
            // Erzeugen eines zufälligen Wertes zwischen 0 und 100 mit zufälligem Vorzeichen
            random_sign = rand() % 10;
            random_sign = (random_sign > 5) ? 1 : -1;
            random_values[si] = random_sign * (rand() % 100);
            difference += random_values[si]; // Messen der tatsächlichen Differenz
            pthread_create(&pthread_array[si], NULL, &change_balance_atomic, &random_values[si]);
        }

        for (size_t si = 0; si < n_threads; si++) {
            pthread_join(pthread_array[si], (void**) &results[si]);
        }
    }

    // Abspeichern des Wertes von balance in lokaler Variable
    const long long final_balance = atomic_load(&balance);

    printf("Ursprünglicher Kontostand: \t%+lld\n"
           "Neuer Kontostand: \t\t%+lld\n"
           "Gemessene Differenz: \t%+lld\n"
           "Tatsächliche Differenz: \t%+lld\n",
           original_balance, final_balance, difference, final_balance - original_balance);

    // Befreien der Arrays
    free(pthread_array);
    free(random_values);
    free(results);
}
