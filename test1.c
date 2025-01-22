#include "supermarket.h"

int main() {
    // Połączenie z pamięcią współdzieloną
    int shm_id = shmget(SHM_KEY, MAX_KASY * sizeof(Kasa), 0600);
    if (shm_id < 0) {
        perror("Nie udało się połączyć z pamięcią współdzieloną");
        exit(1);
    }

    Kasa *kasy = (Kasa *)shmat(shm_id, NULL, 0);

    // Liczenie czynnych kas
    int czynne_kasy = 0;
    for (int i = 0; i < MAX_KASY; i++) {
        if (kasy[i].czynna) {
            czynne_kasy++;
        }
    }

    printf("Test 1: Liczba otwartych kas przy 0 klientach = %d\n", czynne_kasy);

    // Odłączenie od pamięci współdzielonej
    shmdt(kasy);

    return 0;
}