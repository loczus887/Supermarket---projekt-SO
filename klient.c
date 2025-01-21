#include "supermarket.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Użycie: %s <ID klienta>\n", argv[0]);
        exit(1);
    }

    int id_klienta = atoi(argv[1]); // ID klienta przekazane jako argument

    // Połączenie z pamięcią współdzieloną
    int shm_id = shmget(SHM_KEY, MAX_KASY * sizeof(Kasa), 0666);
    if (shm_id < 0) {
        perror("Nie udało się połączyć z pamięcią współdzieloną");
        exit(1);
    }

    Kasa *kasy = (Kasa *)shmat(shm_id, NULL, 0);

    // Połączenie do zmiennej przechowującej liczbę klientów w sklepie
    int shm_klienci_id = shmget(SHM_KEY + 1, sizeof(int), 0666);
    if (shm_klienci_id < 0) {
        perror("Nie udało się połączyć z liczbą klientów w sklepie");
        exit(1);
    }
    int *liczba_klientow = (int *)shmat(shm_klienci_id, NULL, 0);

    // Połączenie do flagi pożaru
    int shm_pozar_id = shmget(SHM_POZAR_KEY, sizeof(int), 0666);
    if (shm_pozar_id < 0) {
        perror("Nie udało się połączyć z pamięcią flagi pożaru");
        exit(1);
    }
    int *pozar = (int *)shmat(shm_pozar_id, NULL, 0);

    // Połączenie do pamięci flagi awarii
    int shm_awaria_id = shmget(SHM_AWARIA_KEY, sizeof(int), 0666);
    if (shm_awaria_id < 0) {
        perror("Nie udało się połączyć z pamięcią flagi awarii");
        exit(1);
    }
    int *awaria = (int *)shmat(shm_awaria_id, NULL, 0);

    srand(time(NULL) ^ getpid());

    while (1) {
        if (__sync_fetch_and_add(liczba_klientow, 0) < MAKS_KLIENTOW) {
            __sync_fetch_and_add(liczba_klientow, 1);
            printf("Klient %d: Wchodzę do sklepu.\n", id_klienta);
            break;
        } else {
            printf("Klient %d: Sklep pełny, czekam na wejście.\n", id_klienta);
            usleep(500000);
        }
    }

    // Klient wybór kasy i zakupy
    int min_idx = -1;
    int min_kolejka = __INT_MAX__;
    while (1) {
        if (*pozar || *awaria) {
            printf("Klient %d: Awaria lub pożar! Opuszczam sklep.\n", id_klienta);
            __sync_fetch_and_sub(liczba_klientow, 1);
            exit(0);
        }

        for (int i = 0; i < MAX_KASY; i++) {
            if (kasy[i].czynna && kasy[i].kolejka < min_kolejka) {
                min_kolejka = kasy[i].kolejka;
                min_idx = i;
            }
        }

        if (min_idx >= 0) {
            __sync_fetch_and_add(&kasy[min_idx].kolejka, 1);
            printf("Klient %d: Wybrałem kasę %d, kolejka = %d\n", id_klienta, min_idx + 1, kasy[min_idx].kolejka);
            sleep(rand() % 5 + 1);
            __sync_fetch_and_sub(&kasy[min_idx].kolejka, 1);
            break;
        }

        usleep(100000);
    }

    __sync_fetch_and_sub(liczba_klientow, 1);
    printf("Klient %d: Opuszczam sklep.\n", id_klienta);
    exit(0);
}