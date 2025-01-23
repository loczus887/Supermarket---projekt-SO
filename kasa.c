#include "supermarket.h"

int main(int argc, char *argv[]) {
    // **Sprawdzenie argumentów**
    if (argc < 2) {
        fprintf(stderr, "Podaj numer kasy jako argument\n");
        exit(1);
    }

    int id = atoi(argv[1]); // ID kasy przekazane jako argument

    // **Połączenie z pamięcią współdzieloną dla kas**
    int shm_id = shmget(SHM_KEY, MAX_KASY * sizeof(Kasa), 0600);
    if (shm_id < 0) {
        perror("Nie udało się połączyć z pamięcią współdzieloną");
        exit(1);
    }
    Kasa *kasy = (Kasa *)shmat(shm_id, NULL, 0);

    // **Główna pętla monitorująca stan kasy**
    while (1) {
        if (kasy[id].czynna) {
            printf("Kasa %d: Kolejka = %d\n", id + 1, kasy[id].kolejka);
        }
        sleep(1); // Odświeżanie statusu co 1 sekundę
        }
    return 0;
}