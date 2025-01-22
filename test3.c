#include "supermarket.h"
#include <pthread.h>

#define LICZBA_KLIENTOW 100   // Liczba klientów w teście
#define LICZBA_ITERACJI 10    // Liczba iteracji symulujących ruch klientów

// Funkcja klienta
void *klient_operacja(void *arg) {
    int id = *(int *)arg;

    // Połączenie z pamięcią współdzieloną
    int shm_id = shmget(SHM_KEY, MAX_KASY * sizeof(Kasa), 0600);
    if (shm_id < 0) {
        perror("Nie udało się połączyć z pamięcią współdzieloną");
        pthread_exit(NULL);
    }

    Kasa *kasy = (Kasa *)shmat(shm_id, NULL, 0);

    for (int iter = 0; iter < LICZBA_ITERACJI; iter++) {
        int wybrana_kasa = rand() % MAX_KASY;

        if (kasy[wybrana_kasa].czynna) {
            // Dodaj do kolejki
            __sync_fetch_and_add(&kasy[wybrana_kasa].kolejka, 1);
            printf("Klient %d: Dołączam do kolejki kasy %d (iteracja %d).\n", id, wybrana_kasa + 1, iter + 1);

            usleep(rand() % 100000); // Symulacja czasu zakupów

            // Usuń z kolejki
            __sync_fetch_and_sub(&kasy[wybrana_kasa].kolejka, 1);
            printf("Klient %d: Opuszczam kolejkę kasy %d (iteracja %d).\n", id, wybrana_kasa + 1, iter + 1);
        } else {
            printf("Klient %d: Kasa %d zamknięta, próbuję ponownie (iteracja %d).\n", id, wybrana_kasa + 1, iter + 1);
        }

        usleep(rand() % 50000); // Losowy czas przed wyborem kolejnej kasy
    }

    shmdt(kasy);
    pthread_exit(NULL);
}

int main() {
    pthread_t klienci[LICZBA_KLIENTOW];
    int ids[LICZBA_KLIENTOW];

    srand(time(NULL));

    // Połączenie z pamięcią współdzieloną
    int shm_id = shmget(SHM_KEY, MAX_KASY * sizeof(Kasa), IPC_CREAT | 0600);
    if (shm_id < 0) {
        perror("Nie udało się utworzyć pamięci współdzielonej");
        exit(1);
    }

    Kasa *kasy = (Kasa *)shmat(shm_id, NULL, 0);

    // Inicjalizacja stanu kas (minimum dwie otwarte)
    for (int i = 0; i < MAX_KASY; i++) {
        kasy[i].czynna = (i < MIN_CZYNNE_KASY) ? 1 : 0;
        kasy[i].kolejka = 0;
        kasy[i].obsluzonych_klientow = 0;
    }

    // Tworzenie klientów
    for (int i = 0; i < LICZBA_KLIENTOW; i++) {
        ids[i] = i + 1;
        pthread_create(&klienci[i], NULL, klient_operacja, &ids[i]);
    }

    // Czekanie na zakończenie wszystkich wątków
    for (int i = 0; i < LICZBA_KLIENTOW; i++) {
        pthread_join(klienci[i], NULL);
    }

    // Sprawdzenie końcowego stanu
    int laczna_kolejka = 0;
    for (int i = 0; i < MAX_KASY; i++) {
        printf("Kasa %d: Kolejka końcowa = %d\n", i + 1, kasy[i].kolejka);
        laczna_kolejka += kasy[i].kolejka;
    }

    if (laczna_kolejka == 0) {
        printf("Test 3: Brak zakleszczeń - wszystkie operacje zakończone poprawnie.\n");
    } else {
        printf("Test 3: Potencjalny deadlock - niewłaściwy stan kolejek.\n");
    }

    shmdt(kasy);
    shmctl(shm_id, IPC_RMID, NULL); // Usunięcie pamięci współdzielonej

    return 0;
}