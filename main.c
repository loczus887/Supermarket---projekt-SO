#include "supermarket.h"
#include <sys/wait.h>

#define MAX_PROCESSES 10000 // Maksymalna liczba procesów klientów

int main() {
    pid_t pid;

    // **Inicjalizacja pamięci współdzielonej dla kas**
    int shm_id = shmget(SHM_KEY, MAX_KASY * sizeof(Kasa), IPC_CREAT | 0600);
    if (shm_id < 0) {
        perror("Nie udało się utworzyć pamięci współdzielonej dla kas");
        exit(1);
    }

    // Mapowanie pamięci współdzielonej
    Kasa *kasy = (Kasa *)shmat(shm_id, NULL, 0);

    // **Inicjalizacja stanu kas**
    for (int i = 0; i < MAX_KASY; i++) {
        kasy[i].czynna = (i < MIN_CZYNNE_KASY) ? 1 : 0; // Minimalna liczba czynnych kas
        kasy[i].kolejka = 0;                            // Początkowa długość kolejki
        kasy[i].obsluzonych_klientow = 0;               // Początkowa liczba obsłużonych klientów
    }

    // **Inicjalizacja pamięci współdzielonej dla liczby klientów**
    int shm_klienci_id = shmget(SHM_KEY + 1, sizeof(int), IPC_CREAT | 0600);
    if (shm_klienci_id < 0) {
        perror("Nie udało się utworzyć pamięci współdzielonej dla liczby klientów");
        exit(1);
    }
    int *liczba_klientow = (int *)shmat(shm_klienci_id, NULL, 0);
    *liczba_klientow = 0; // Początkowa liczba klientów

    // **Inicjalizacja pamięci współdzielonej dla liczby procesów**
    int shm_processes_id = shmget(SHM_PROCESSES_KEY, sizeof(int), IPC_CREAT | 0600);
    if (shm_processes_id < 0) {
        perror("Nie udało się utworzyć pamięci współdzielonej dla liczby procesów");
        exit(1);
    }
    int *liczba_procesow = (int *)shmat(shm_processes_id, NULL, 0);
    *liczba_procesow = 0; // Początkowa liczba procesów

    // **Inicjalizacja pamięci współdzielonej dla flag pożaru i awarii**
    int shm_pozar_id = shmget(SHM_POZAR_KEY, sizeof(int), IPC_CREAT | 0600);
    if (shm_pozar_id < 0) {
        perror("Nie udało się utworzyć pamięci współdzielonej dla flagi pożaru");
        exit(1);
    }
    int *pozar = (int *)shmat(shm_pozar_id, NULL, 0);
    *pozar = 0; // Początkowa wartość flagi pożaru

    int shm_awaria_id = shmget(SHM_AWARIA_KEY, sizeof(int), IPC_CREAT | 0600);
    if (shm_awaria_id < 0) {
        perror("Nie udało się utworzyć pamięci współdzielonej dla flagi awarii");
        exit(1);
    }
    int *awaria = (int *)shmat(shm_awaria_id, NULL, 0);
    *awaria = 0; // Początkowa wartość flagi awarii

    // **Tworzenie procesu dla kierownika**
    pid = fork();
    if (pid == 0) {
        execl("./kierownik", "./kierownik", NULL);
        perror("Nie udało się uruchomić kierownika");
        exit(1);
    }

    // **Tworzenie procesu dla strażaka**
    pid = fork();
    if (pid == 0) {
        execl("./strazak", "./strazak", NULL);
        perror("Nie udało się uruchomić strażaka");
        exit(1);
    }

    int id_klienta = 1; // ID dla klientów

    // **Główna pętla tworzenia klientów**
    while (1) {
        // Kontrola limitu procesów klientów
        if (*liczba_procesow >= MAX_PROCESSES) {
            printf("Osiągnięto maksymalną liczbę procesów klientów (%d). Czekam na wolne miejsce...\n", MAX_PROCESSES);
            usleep(100000); // Oczekiwanie na zwolnienie miejsca
            continue;
        }

        // Tworzenie nowego procesu klienta
        pid = fork();
        if (pid == 0) {
            char buf[10];
            sprintf(buf, "%d", id_klienta);
            execl("./klient", "./klient", buf, NULL);
            perror("Nie udało się uruchomić klienta");
            exit(1);
        } else if (pid > 0) {
            __sync_fetch_and_add(liczba_procesow, 1); // Aktualizacja liczby procesów
        } else {
            perror("Fork nie powiódł się");
            exit(1);
        }

        id_klienta++; // Inkrementacja ID klienta

        // Obsługa zakończonych procesów klientów
        int status;
        while (waitpid(-1, &status, WNOHANG) > 0) {
            __sync_fetch_and_sub(liczba_procesow, 1); // Aktualizacja liczby procesów
        }

        usleep(100000); // Oczekiwanie przed utworzeniem kolejnego klienta
    }
    return 0;
}
