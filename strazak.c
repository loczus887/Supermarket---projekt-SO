#include "supermarket.h"

void strazak_obsluga(int sig) {
    key_t key = ftok("supermarket", 65);
    int msgid = msgget(key, 0666 | IPC_CREAT);

    printf("Strażak: Pożar! Zamykam supermarket.\n");
    msgctl(msgid, IPC_RMID, NULL);    //Usuwanie kolejki komunikatów
    exit(0);
}

int main() {
    signal(SIGINT, strazak_obsluga);
    while (1) {
        pause();
    }
    return 0;
}
#include "supermarket.h"

void strazak_obsluga(int sig) {
    // Połączenie do pamięci flagi pożaru
    int shm_pozar_id = shmget(SHM_POZAR_KEY, sizeof(int), 0666);
    if (shm_pozar_id < 0) {
        perror("Nie udało się połączyć z pamięcią flagi pożaru");
        exit(1);
    }
    int *pozar = (int *)shmat(shm_pozar_id, NULL, 0);

    *pozar = 1; // Ustawienie flagi pożaru
    printf("Strażak: Pożar! Wszyscy klienci muszą opuścić sklep.\n");
    shmdt(pozar); // Odłączenie od pamięci

    exit(0); // Zakończenie procesu strażaka
}

int main() {
    // Inicjalizacja pamięci flagi pożaru
    int shm_pozar_id = shmget(SHM_POZAR_KEY, sizeof(int), IPC_CREAT | 0666);
    if (shm_pozar_id < 0) {
        perror("Nie udało się utworzyć pamięci flagi pożaru");
        exit(1);
    }
    int *pozar = (int *)shmat(shm_pozar_id, NULL, 0);
    *pozar = 0; // Początkowa wartość - brak pożaru

    signal(SIGINT, strazak_obsluga);
    while (1) {
        pause(); // Czekanie na sygnał
    }
}