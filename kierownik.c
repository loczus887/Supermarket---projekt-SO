#include "supermarket.h"

int main() {
    // Połączenie z pamięcią współdzieloną
    int shm_id = shmget(SHM_KEY, MAX_KASY * sizeof(Kasa), 0666);
    if (shm_id < 0) {
        perror("Nie udało się połączyć z pamięcią współdzieloną");
        exit(1);
    }

    Kasa *kasy = (Kasa *)shmat(shm_id, NULL, 0);

    while (1) {
        int liczba_klientow = 0;
        int czynne_kasy = 0;

        // Oblicz liczbę klientów i aktywnych kas
        for (int i = 0; i < MAX_KASY; i++) {
            if (kasy[i].czynna) {
                liczba_klientow += kasy[i].kolejka;
                czynne_kasy++;
            }
        }

        // Otwieraj nowe kasy, gdy przekroczono próg klientów
      if (czynne_kasy < MAX_KASY && liczba_klientow > KLIENT_PER_KASA * czynne_kasy) {
            for (int i = 0; i < MAX_KASY; i++) {
                if (!kasy[i].czynna) {
                    kasy[i].czynna = 1;
                    czynne_kasy++;
                    printf("Kierownik: Otwieram kasę %d (klientów: %d, czynne kasy: %d)\n",
                           i + 1, liczba_klientow, czynne_kasy);
                    break;
                }
            }
        }

        // Zamykaj kasy, gdy liczba klientów spadnie
        if (czynne_kasy > MIN_CZYNNE_KASY &&
            liczba_klientow <= KLIENT_PER_KASA * (czynne_kasy - 1)) {
            for (int i = MAX_KASY - 1; i >= 0; i--) {
                if (kasy[i].czynna) {
                    kasy[i].czynna = 0;
                    czynne_kasy--;
                    printf("Kierownik: Zamykam kasę %d (klientów: %d, czynne kasy: %d)\n",
                           i + 1, liczba_klientow, czynne_kasy);
                    break;
                }
            }
        }

        // Częstsze sprawdzanie stanu
        usleep(20000); // Sprawdzaj stan co 20 ms
    }
    return 0;
}