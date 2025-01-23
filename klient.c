#include "supermarket.h"
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

sem_t *sem_klient;  // Semafor do synchronizacji liczby klientów

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Użycie: %s <ID klienta>\n", argv[0]);
        exit(1);
    }

    int id_klienta = atoi(argv[1]); // ID klienta przekazane jako argument

    // Połączenie z pamięcią współdzieloną
    int shm_id = shmget(SHM_KEY, MAX_KASY * sizeof(Kasa), 0600);
    if (shm_id < 0) {
        perror("Nie udało się połączyć z pamięcią współdzieloną");
        exit(1);
    }

    Kasa *kasy = (Kasa *)shmat(shm_id, NULL, 0);

    // Połączenie do zmiennej przechowującej liczbę klientów w sklepie
    int shm_klienci_id = shmget(SHM_KEY + 1, sizeof(int), 0600);
    if (shm_klienci_id < 0) {
        perror("Nie udało się połączyć z liczbą klientów w sklepie");
        exit(1);
    }
    int *liczba_klientow = (int *)shmat(shm_klienci_id, NULL, 0);

    // Połączenie do flagi pożaru
    int shm_pozar_id = shmget(SHM_POZAR_KEY, sizeof(int), 0600);
    if (shm_pozar_id < 0) {
        perror("Nie udało się połączyć z pamięcią flagi pożaru");
        exit(1);
    }
    int *pozar = (int *)shmat(shm_pozar_id, NULL, 0);

    // Połączenie do pamięci flagi awarii
    int shm_awaria_id = shmget(SHM_AWARIA_KEY, sizeof(int), 0600);
    if (shm_awaria_id < 0) {
        perror("Nie udało się połączyć z pamięcią flagi awarii");
        exit(1);
    }
    int *awaria = (int *)shmat(shm_awaria_id, NULL, 0);

    srand(time(NULL) ^ getpid());

    // Inicjalizacja semafora
    sem_klient = sem_open("/sem_klient", O_CREAT, 0600, 1); // Semafor inicjalny na 1 (możliwość dostępu)
    if (sem_klient == SEM_FAILED) {
        perror("Nie udało się otworzyć semafora");
        exit(1);
    }

    while (1) {
        sleep(1);
        // Zatrzymanie procesu w przypadku pełnego sklepu
        sem_wait(sem_klient);  // Czekaj na dostęp do zmiennej liczba_klientow
        if (*liczba_klientow < MAKS_KLIENTOW) {
            __sync_fetch_and_add(liczba_klientow, 1);
            printf("Klient %d: Wchodzę do sklepu.\n", id_klienta);
            sem_post(sem_klient);  // Zwolnienie semafora
            break;
        } else {
            printf("Klient %d: Sklep pełny, czekam na wejście.\n", id_klienta);
            sem_post(sem_klient);  // Zwolnienie semafora
            usleep(500000);
        }
    }

    // Klient wybór kasy i zakupy
    int min_idx = -1;
    int min_kolejka = __INT_MAX__;
    while (1) {
        sleep(1);
        if (*pozar || *awaria) {
            printf("Klient %d: Awaria lub pożar! Opuszczam sklep.\n", id_klienta);
            sem_wait(sem_klient);  // Zablokowanie dostępu do liczby_klientow
            __sync_fetch_and_sub(liczba_klientow, 1);
            sem_post(sem_klient);  // Zwolnienie semafora
            exit(0);
        }

        for (int i = 0; i < MAX_KASY; i++) {
            if (kasy[i].czynna && !kasy[i].do_zamkniecia && kasy[i].kolejka < min_kolejka) {
                min_kolejka = kasy[i].kolejka;
                min_idx = i;
            }
        }

        if (min_idx >= 0) {
            sem_wait(sem_klient);  // Zablokowanie dostępu do zmiennej kolejki
            __sync_fetch_and_add(&kasy[min_idx].kolejka, 1);
            printf("Klient %d: Wybrałem kasę %d, kolejka = %d\n", id_klienta, min_idx + 1, kasy[min_idx].kolejka);
            sem_post(sem_klient);  // Zwolnienie semafora

            sleep(rand() % 10 + 1); // Czas spędzony przy kasie

            sem_wait(sem_klient);  // Zablokowanie dostępu do zmiennej kolejki
            __sync_fetch_and_sub(&kasy[min_idx].kolejka, 1);
            __sync_fetch_and_add(&kasy[min_idx].obsluzonych_klientow, 1);
            sem_post(sem_klient);  // Zwolnienie semafora
            break;
        }

        usleep(100000);
    }

    sem_wait(sem_klient);  // Zablokowanie dostępu do liczby_klientow
    __sync_fetch_and_sub(liczba_klientow, 1);
    sem_post(sem_klient);  // Zwolnienie semafora

    printf("Klient %d: Opuszczam sklep.\n", id_klienta);

    // Zamknięcie semafora
    sem_close(sem_klient);
    exit(0);
}
