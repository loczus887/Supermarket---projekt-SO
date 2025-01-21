#include "supermarket.h"
#include <sys/wait.h>

int main() {
    pid_t pid;

    // Połączenie z pamięcią współdzieloną
    int shm_id = shmget(SHM_KEY, MAX_KASY * sizeof(Kasa), IPC_CREAT | 0666);
    if (shm_id < 0) {
        perror("Nie udało się utworzyć pamięci współdzielonej");
        exit(1);
    }

    Kasa *kasy = (Kasa *)shmat(shm_id, NULL, 0);

    // Inicjalizacja pamięci współdzielonej
    for (int i = 0; i < MAX_KASY; i++) {
        kasy[i].czynna = (i < MIN_CZYNNE_KASY) ? 1 : 0;
        kasy[i].kolejka = 0;
        kasy[i].obsluzonych_klientow = 0;
    }

    // Inicjalizacja flagi pożaru
    int shm_pozar_id = shmget(SHM_POZAR_KEY, sizeof(int), IPC_CREAT | 0666);
    if (shm_pozar_id < 0) {
        perror("Nie udało się utworzyć pamięci flagi pożaru");
        exit(1);
    }
    int *pozar = (int *)shmat(shm_pozar_id, NULL, 0);
    *pozar = 0;

    // Inicjalizacja pamięci flagi awarii
    int shm_awaria_id = shmget(SHM_AWARIA_KEY, sizeof(int), IPC_CREAT | 0666);
    if (shm_awaria_id < 0) {
        perror("Nie udało się utworzyć pamięci flagi awarii");
        exit(1);
    }
    int *awaria = (int *)shmat(shm_awaria_id, NULL, 0);
    *awaria = 0; // Początkowa wartość - brak awarii

    // Tworzenie kierownika
    pid = fork();
    if (pid == 0) {
        execl("./kierownik", "./kierownik", NULL);
        perror("Nie udało się uruchomić kierownika");
        exit(1);
    }

    // Tworzenie strażaka
    pid = fork();
    if (pid == 0) {
        execl("./strazak", "./strazak", NULL);
        perror("Nie udało się uruchomić strażaka");
        exit(1);
    }

    int id_klienta = 1;

    // Tworzenie klientów w pętli
    while (1) {
        if ((pid = fork()) == 0) {
            char buf[10];
            sprintf(buf, "%d", id_klienta);
            execl("./klient", "./klient", buf, NULL);
            perror("Nie udało się uruchomić klienta");
            exit(1);
        }

        id_klienta++;

        int status;
        while (waitpid(-1, &status, WNOHANG) > 0) {
            printf("Proces klienta zakończył się.\n");
        }

        usleep(100000);
    }

    return 0;
}