#include "supermarket.h"

int main() {
    // Połączenie z pamięcią współdzieloną
    int shm_id = shmget(SHM_KEY, MAX_KASY * sizeof(Kasa), 0666);
    if (shm_id < 0) {
        perror("Nie udało się połączyć z pamięcią współdzieloną");
        exit(1);
    }

    Kasa *kasy = (Kasa *)shmat(shm_id, NULL, 0);

    // Połączenie do zmiennej przechowującej liczbę klientów w sklepie
    int shm_klienci_id = shmget(SHM_KEY + 1, sizeof(int), IPC_CREAT | 0666);
    if (shm_klienci_id < 0) {
        perror("Nie udało się utworzyć pamięci dla liczby klientów w sklepie");
        exit(1);
    }
    int *liczba_klientow = (int *)shmat(shm_klienci_id, NULL, 0);
    *liczba_klientow = 0; 

    while (1) {
        int czynne_kasy = 0;

        // Liczenie czynnych kas
        for (int i = 0; i < MAX_KASY; i++) {
            if (kasy[i].czynna) {
                czynne_kasy++;
            }
        }

        printf("Kierownik: liczba_klientow = %d, czynne_kasy = %d\n",
               *liczba_klientow, czynne_kasy);
        fflush(stdout);

        // Otwieranie nowych kas
        while (*liczba_klientow > KLIENT_PER_KASA * czynne_kasy && czynne_kasy < MAX_KASY) {
            for (int i = 0; i < MAX_KASY; i++) {
                if (!kasy[i].czynna) {
                    kasy[i].czynna = 1;
                    czynne_kasy++;
                    break;
                }
            }
        }

        // Zamykanie kas
        while (czynne_kasy > MIN_CZYNNE_KASY &&
               *liczba_klientow <= KLIENT_PER_KASA * (czynne_kasy - 1)) {
            for (int i = MAX_KASY - 1; i >= 0; i--) {
                if (kasy[i].czynna) {
                    kasy[i].czynna = 0;
                    czynne_kasy--;
                    printf("Kierownik: Zamykam kasę %d (klientów: %d, czynne kasy: %d)\n",
                           i + 1, *liczba_klientow, czynne_kasy);
                    fflush(stdout);
                    break;
                }
            }
        }

        usleep(100000); // Sprawdzanie co 100 ms
    }

    return 0;
}