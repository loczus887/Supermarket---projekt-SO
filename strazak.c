#include "supermarket.h"
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

// Funkcja do czyszczenia zasobów IPC
void wyczysc_ipc() {
    shmctl(shmget(SHM_KEY, MAX_KASY * sizeof(Kasa), 0600), IPC_RMID, NULL);
    shmctl(shmget(SHM_KEY + 1, sizeof(int), 0600), IPC_RMID, NULL);
    shmctl(shmget(SHM_POZAR_KEY, sizeof(int), 0600), IPC_RMID, NULL);
    shmctl(shmget(SHM_AWARIA_KEY, sizeof(int), 0600), IPC_RMID, NULL);
    shmctl(shmget(SHM_PROCESSES_KEY, sizeof(int), 0600), IPC_RMID, NULL);
    printf("Strażak: Wyczyszczono zasoby IPC.\n");
}


void zapisz_raport() {
    int fd = creat("raport_kasy.txt", S_IRUSR | S_IWUSR);
    if (fd < 0) {
        perror("Nie udało się utworzyć pliku raportu");
        return;
    }

    int shm_id = shmget(SHM_KEY, MAX_KASY * sizeof(Kasa), 0600);
    if (shm_id < 0) {
        perror("Nie udało się połączyć z pamięcią współdzieloną");
        close(fd);
        return;
    }

    Kasa *kasy = (Kasa *)shmat(shm_id, NULL, 0);

    char buffer[256];
    int len = snprintf(buffer, sizeof(buffer), "Raport dzienny - obsłużeni klienci:\n");
    if (write(fd, buffer, len) < 0) {
        perror("Nie udało się zapisać do pliku");
        shmdt(kasy);
        close(fd);
        return;
    }

    for (int i = 0; i < MAX_KASY; i++) {
        len = snprintf(buffer, sizeof(buffer), "Kasa %d: %d klientów\n", i + 1, kasy[i].obsluzonych_klientow);
        if (write(fd, buffer, len) < 0) {
            perror("Nie udało się zapisać do pliku");
            shmdt(kasy);
            close(fd);
            return;
        }
    }

    shmdt(kasy);
    close(fd);
    printf("Strażak: Raport zapisany do pliku 'raport_kasy.txt'.\n");
}


// Funkcja obsługi sygnału pożaru
void strazak_obsluga_pozaru(int sig) {
    int shm_pozar_id = shmget(SHM_POZAR_KEY, sizeof(int), 0600);
    if (shm_pozar_id < 0) {
        perror("Nie udało się połączyć z pamięcią flagi pożaru");
        exit(1);
    }
    int *pozar = (int *)shmat(shm_pozar_id, NULL, 0);

    *pozar = 1; // Ustawienie flagi pożaru
    printf("Strażak: Pożar! Wszyscy klienci muszą opuścić sklep.\n");
    shmdt(pozar);

    // Czyszczenie zasobów IPC i zapis raportu
    zapisz_raport();
    wyczysc_ipc();
    exit(0);
}

// Funkcja obsługi sygnału awarii prądu
void strazak_obsluga_awarii(int sig) {
    int shm_awaria_id = shmget(SHM_AWARIA_KEY, sizeof(int), 0600);
    if (shm_awaria_id < 0) {
        perror("Nie udało się połączyć z pamięcią flagi awarii");
        exit(1);
    }
    int *awaria = (int *)shmat(shm_awaria_id, NULL, 0);

    *awaria = 1; // Ustawienie flagi awarii
    printf("Strażak: Awaria prądu! Wszyscy klienci muszą opuścić sklep.\n");
    shmdt(awaria);

    // Czyszczenie zasobów IPC i zapis raportu
    zapisz_raport();
    wyczysc_ipc();
    exit(0);
}

int main() {
    // Inicjalizacja pamięci flagi pożaru
    int shm_pozar_id = shmget(SHM_POZAR_KEY, sizeof(int), IPC_CREAT | 0600);
    if (shm_pozar_id < 0) {
        perror("Nie udało się utworzyć pamięci flagi pożaru");
        exit(1);
    }
    int *pozar = (int *)shmat(shm_pozar_id, NULL, 0);
    *pozar = 0;

    // Inicjalizacja pamięci flagi awarii
    int shm_awaria_id = shmget(SHM_AWARIA_KEY, sizeof(int), IPC_CREAT | 0600);
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