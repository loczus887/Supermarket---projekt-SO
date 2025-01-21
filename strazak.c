#include "supermarket.h"
#include <signal.h>

// Funkcja do czyszczenia zasobów IPC
void wyczysc_ipc() {
    shmctl(shmget(SHM_KEY, MAX_KASY * sizeof(Kasa), 0666), IPC_RMID, NULL);
    shmctl(shmget(SHM_KEY + 1, sizeof(int), 0666), IPC_RMID, NULL);
    shmctl(shmget(SHM_POZAR_KEY, sizeof(int), 0666), IPC_RMID, NULL);
    shmctl(shmget(SHM_AWARIA_KEY, sizeof(int), 0666), IPC_RMID, NULL);
    printf("Strażak: Wyczyszczono zasoby IPC.\n");
}

// Funkcja obsługi sygnału pożaru
void strazak_obsluga_pozaru(int sig) {
    int shm_pozar_id = shmget(SHM_POZAR_KEY, sizeof(int), 0666);
    if (shm_pozar_id < 0) {
        perror("Nie udało się połączyć z pamięcią flagi pożaru");
        exit(1);
    }
    int *pozar = (int *)shmat(shm_pozar_id, NULL, 0);

    *pozar = 1; // Ustawienie flagi pożaru
    printf("Strażak: Pożar! Wszyscy klienci muszą opuścić sklep.\n");
    shmdt(pozar);

    // Czyszczenie zasobów IPC
    wyczysc_ipc();
    exit(0);
}

// Funkcja obsługi sygnału awarii prądu
void strazak_obsluga_awarii(int sig) {
    int shm_awaria_id = shmget(SHM_AWARIA_KEY, sizeof(int), 0666);
    if (shm_awaria_id < 0) {
        perror("Nie udało się połączyć z pamięcią flagi awarii");
        exit(1);
    }
    int *awaria = (int *)shmat(shm_awaria_id, NULL, 0);

    *awaria = 1; // Ustawienie flagi awarii
    printf("Strażak: Awaria prądu! Wszyscy klienci muszą opuścić sklep.\n");
    shmdt(awaria);

    // Czyszczenie zasobów IPC
    wyczysc_ipc();
    exit(0);
}

int main() {
    // Inicjalizacja pamięci flagi pożaru
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
    *awaria = 0;

    // Ustawienie obsługi sygnałów
    signal(SIGINT, strazak_obsluga_pozaru); // CTRL+C - pożar
    signal(SIGQUIT, strazak_obsluga_awarii); // CTRL+\ - awaria prądu

    printf("Strażak: Wciśnij CTRL+C dla pożaru lub CTRL+\\ dla awarii prądu.\n");

    // Czekanie na sygnały
    while (1) {
        pause(); // Oczekiwanie na sygnał
    }

    return 0;
}
