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
    srand(time(NULL) ^ getpid());

    while (1) {
        int min_idx = -1;
        int min_kolejka = __INT_MAX__;

        // Znajdź kasę z najkrótszą kolejką
        for (int i = 0; i < MAX_KASY; i++) {
            if (kasy[i].czynna && kasy[i].kolejka < min_kolejka) {
                min_kolejka = kasy[i].kolejka;
                min_idx = i;
            }
        }

        if (min_idx >= 0) {
            __sync_fetch_and_add(&kasy[min_idx].kolejka, 1); // Atomowe zwiększenie kolejki
            printf("Klient %d: Wybrałem kasę %d, kolejka = %d\n", id_klienta, min_idx + 1, kasy[min_idx].kolejka);
            sleep(rand() % 3 + 1); // Symulacja zakupów (1-3 sekundy)
            __sync_fetch_and_sub(&kasy[min_idx].kolejka, 1); // Atomowe zmniejszenie kolejki
            printf("Klient %d: Opuszczam kasę %d\n", id_klienta, min_idx + 1);
            break; 
        }

        usleep(rand() % 500000 + 100000); 
    }
    return 0;
}