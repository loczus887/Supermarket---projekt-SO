#include "supermarket.h"
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>

// **Funkcja czyszcząca zasoby IPC**
void wyczysc_ipc() {
    // Odczytanie semafora z pamięci współdzielonej
    int shm_sem_id = shmget(SHM_PROCESSES_KEY, sizeof(int), 0600);
    if (shm_sem_id < 0) {
        perror("Nie udało się połączyć z pamięcią współdzieloną dla semafora");
        exit(1);
    }

    int *sem_id_ptr = (int *)shmat(shm_sem_id, NULL, 0);
    int sem_id = *sem_id_ptr;  // Odczytujemy semafor z pamięci współdzielonej
    shmdt(sem_id_ptr);

    // Usuwanie pamięci współdzielonej
    shmctl(shmget(SHM_KEY, MAX_KASY * sizeof(Kasa), 0600), IPC_RMID, NULL);
    shmctl(shmget(SHM_KEY + 1, sizeof(int), 0600), IPC_RMID, NULL);
    shmctl(shmget(SHM_POZAR_KEY, sizeof(int), 0600), IPC_RMID, NULL);
    shmctl(shmget(SHM_AWARIA_KEY, sizeof(int), 0600), IPC_RMID, NULL);
    shmctl(shmget(SHM_PROCESSES_KEY, sizeof(int), 0600), IPC_RMID, NULL);
    // Usuwanie semafora
    semctl(sem_id, 0, IPC_RMID);  
    printf("Strażak: Wyczyszczono zasoby IPC.\n");
}

// **Funkcja zapisująca raport o stanie kas**
void zapisz_raport() {
    int fd = creat("raport_kasy.txt", S_IRUSR | S_IWUSR); // Tworzenie pliku raportu
    if (fd < 0) {
        perror("Nie udało się utworzyć pliku raportu");
        return;
    }

    // Pobranie pamięci współdzielonej dla kas
    int shm_id = shmget(SHM_KEY, MAX_KASY * sizeof(Kasa), 0600);
    if (shm_id < 0) {
        perror("Nie udało się połączyć z pamięcią współdzieloną");
        close(fd);
        return;
    }

    Kasa *kasy = (Kasa *)shmat(shm_id, NULL, 0);

    // Zapis nagłówka do pliku
    char buffer[256];
    int len = snprintf(buffer, sizeof(buffer), "Raport dzienny - obsłużeni klienci:\n");
    if (write(fd, buffer, len) < 0) {
        perror("Nie udało się zapisać do pliku");
        shmdt(kasy);
        close(fd);
        return;
    }

    // Zapis danych każdej kasy do pliku
    for (int i = 0; i < MAX_KASY; i++) {
        len = snprintf(buffer, sizeof(buffer), "Kasa %d: %d klientów\n", i + 1, kasy[i].obsluzonych_klientow);
        if (write(fd, buffer, len) < 0) {
            perror("Nie udało się zapisać do pliku");
            shmdt(kasy);
            close(fd);
            return;
        }
    }

    // Zwolnienie pamięci współdzielonej
    shmdt(kasy);
    close(fd);
    printf("Strażak: Raport zapisany do pliku 'raport_kasy.txt'.\n");
}

// **Obsługa sygnału pożaru**
void strazak_obsluga_pozaru(int sig) {
    // **Ustawienie flagi pożaru**
    int shm_pozar_id = shmget(SHM_POZAR_KEY, sizeof(int), 0600);
    if (shm_pozar_id < 0) {
        perror("Nie udało się połączyć z pamięcią współdzieloną dla flagi pożaru");
        exit(1);
    }
    int *pozar = (int *)shmat(shm_pozar_id, NULL, 0);
    *pozar = 1; // Ustawienie flagi pożaru
    shmdt(pozar);

    printf("Strażak: Pożar! Wszyscy klienci muszą opuścić sklep.\n");

    // **Zapis raportu i czyszczenie zasobów**
    zapisz_raport();
    wyczysc_ipc();
    exit(0);
}

// **Obsługa sygnału awarii prądu**
void strazak_obsluga_awarii(int sig) {
    // **Ustawienie flagi awarii**
    int shm_awaria_id = shmget(SHM_AWARIA_KEY, sizeof(int), 0600);
    if (shm_awaria_id < 0) {
        perror("Nie udało się połączyć z pamięcią współdzieloną dla flagi awarii");
        exit(1);
    }
    int *awaria = (int *)shmat(shm_awaria_id, NULL, 0);
    *awaria = 1; // Ustawienie flagi awarii
    printf("Strażak: Awaria prądu! Wszyscy klienci muszą opuścić sklep.\n");
    shmdt(awaria);

    // **Zapis raportu i czyszczenie zasobów**
    zapisz_raport();
    wyczysc_ipc();
    exit(0);
}

int main() {
    // **Inicjalizacja pamięci dla flagi pożaru**
    int shm_pozar_id = shmget(SHM_POZAR_KEY, sizeof(int), IPC_CREAT | 0600);
    if (shm_pozar_id < 0) {
        perror("Nie udało się utworzyć pamięci dla flagi pożaru");
        exit(1);
    }
    int *pozar = (int *)shmat(shm_pozar_id, NULL, 0);
    *pozar = 0; // Flaga pożaru ustawiona na brak pożaru

    // **Inicjalizacja pamięci dla flagi awarii**
    int shm_awaria_id = shmget(SHM_AWARIA_KEY, sizeof(int), IPC_CREAT | 0600);
    if (shm_awaria_id < 0) {
        perror("Nie udało się utworzyć pamięci dla flagi awarii");
        exit(1);
    }
    int *awaria = (int *)shmat(shm_awaria_id, NULL, 0);
    *awaria = 0; // Flaga awarii ustawiona na brak awarii

    // **Ustawienie obsługi sygnałów**
    signal(SIGINT, strazak_obsluga_pozaru);  // CTRL+C - Pożar
    signal(SIGQUIT, strazak_obsluga_awarii); // CTRL+\ - Awaria prądu

    printf("Strażak: Wciśnij CTRL+C dla pożaru lub CTRL+\\ dla awarii prądu.\n");

    // **Główna pętla oczekiwania na sygnały**
    while (1) {
        pause(); // Czekanie na sygnały
    }

    return 0;
}
