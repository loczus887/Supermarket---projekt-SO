#include "supermarket.h"
#include <sys/wait.h>

int main() {
    pid_t pid;

    // Inicjalizacja pamięci współdzielonej
    int shm_id = shmget(SHM_KEY, MAX_KASY * sizeof(Kasa), IPC_CREAT | 0666);
    if (shm_id < 0) {
        perror("Nie udało się utworzyć pamięci współdzielonej");
        exit(1);
    }

    Kasa *kasy = (Kasa *)shmat(shm_id, NULL, 0);

    // Inicjalizacja kas
    for (int i = 0; i < MAX_KASY; i++) {
        kasy[i].czynna = (i < MIN_CZYNNE_KASY) ? 1 : 0; 
        kasy[i].kolejka = 0;
    }

    printf("Uruchamianie kierownika...\n");
    if ((pid = fork()) == 0) {
        execl("./kierownik", "./kierownik", NULL);
        perror("Nie udało się uruchomić procesu kierownika");
        exit(1);
    }

    printf("Uruchamianie klientów...\n");
    for (int i = 0; i < 10000; i++) {
        if ((pid = fork()) == 0) {
            char buf[10];
            sprintf(buf, "%d", i + 1); // ID klienta
            execl("./klient", "./klient", buf, NULL);
            perror("Nie udało się uruchomić procesu klienta");
            exit(1);
        }
        usleep(5000); // Odstęp między uruchamianiem klientów (5 ms)
    }
    //Tworzenie nowych keintów, tutaj max 10000 popracuj nad tym
    for (int i = 0; i < 10000; i++) {
        wait(NULL);
    }

    printf("Wszyscy klienci zakończyli zakupy.\n");

    return 0;
}