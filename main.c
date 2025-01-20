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
        kasy[i].czynna = (i < MIN_CZYNNE_KASY) ? 1 : 0; // Pierwsze 2 kasy otwarte
        kasy[i].kolejka = 0;
    }

    // Tworzenie kierownika
    pid = fork();
    if (pid == 0) {
        execl("./kierownik", "./kierownik", NULL);
        perror("Nie udało się uruchomić kierownika");
        exit(1);
    }

    //Zmienna dla ID klientów
    int id_klienta = 1;

    // Tworzenie klientów w pętli
    while (1) {
        if ((pid = fork()) == 0) {
            char buf[10];
            sprintf(buf, "%d", id_klienta); // Przekazanie ID klienta do nowego procesu
            execl("./klient", "./klient", buf, NULL);
            perror("Nie udało się uruchomić klienta");
            exit(1);
        }

        id_klienta++; 

        // Oczekiwanie na zakończone procesy klientów
        int status;
        while (waitpid(-1, &status, WNOHANG) > 0) {
            printf("Proces klienta zakończył się.\n");
        }

        usleep(100000); // Odstęp między generowaniem klientów
    }

    return 0;
}