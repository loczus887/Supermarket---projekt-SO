#include "supermarket.h"
#include <pthread.h>

#define LICZBA_KLIENTOW_TEST 50 // Liczba klientów w teście
#define NUMER_KASY_TEST 0       // Testujemy pierwszą kasę

void *klient_operacja(void *arg) {
    int id = *(int *)arg;

    // Połączenie z pamięcią współdzieloną
    int shm_id = shmget(SHM_KEY, MAX_KASY * sizeof(Kasa), 0600);
    if (shm_id < 0) {
        perror("Nie udało się połączyć z pamięcią współdzieloną");
        pthread_exit(NULL);
    }

    Kasa *kasy = (Kasa *)shmat(shm_id, NULL, 0);

    // Symulacja wejścia do kolejki
    printf("Klient %d: Dodaję się do kolejki kasy %d.\n", id, NUMER_KASY_TEST + 1);
    __sync_fetch_and_add(&kasy[NUMER_KASY_TEST].kolejka, 1);
    usleep(rand() % 100000); // Losowy czas przebywania w kolejce

    // Symulacja opuszczenia kolejki
    printf("Klient %d: Opuszczam kolejkę kasy %d.\n", id, NUMER_KASY_TEST + 1);
    __sync_fetch_and_sub(&kasy[NUMER_KASY_TEST].kolejka, 1);

    shmdt(kasy);
    pthread_exit(NULL);
}

int main() {
    pthread_t klienci[LICZBA_KLIENTOW_TEST];
    int ids[LICZBA_KLIENTOW_TEST];

    srand(time(NULL));

    // Połączenie z pamięcią współdzieloną
    int shm_id = shmget(SHM_KEY, MAX_KASY * sizeof(Kasa), IPC_CREAT | 0600);
    if (shm_id < 0) {
        perror("Nie udało się utworzyć pamięci współdzielonej");
        exit(1);
    }

    Kasa *kasy = (Kasa *)shmat(shm_id, NULL, 0);
    kasy[NUMER_KASY_TEST].kolejka = 0;

    // Tworzenie wątków klientów
    for (int i = 0; i < LICZBA_KLIENTOW_TEST; i++) {
        ids[i] = i + 1;
        pthread_create(&klienci[i], NULL, klient_operacja, &ids[i]);
    }

    // Czekanie na zakończenie wszystkich wątków
    for (int i = 0; i < LICZBA_KLIENTOW_TEST; i++) {
        pthread_join(klienci[i], NULL);
    }

    // Sprawdzenie końcowego stanu kolejki
    printf("Test 2: Końcowa liczba klientów w kolejce kasy %d = %d\n", NUMER_KASY_TEST + 1, kasy[NUMER_KASY_TEST].kolejka);

    // Oczekiwany wynik: kolejka = 0
    if (kasy[NUMER_KASY_TEST].kolejka == 0) {
        printf("Test 2: Synchronizacja poprawna - brak konfliktów.\n");
    } else {
        printf("Test 2: Błąd synchronizacji - kolejka != 0.\n");
    }

    shmdt(kasy);
    shmctl(shm_id, IPC_RMID, NULL); // Usunięcie pamięci współdzielonej

    return 0;
}