#include "supermarket.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

// Funkcja sprawdzająca, czy pamięć współdzielona istnieje
int sprawdz_pamiec_wspoldzielona(key_t key) {
    int shm_id = shmget(key, 0, 0600);
    if (shm_id < 0) {
        printf("Pamięć współdzielona z kluczem %d została poprawnie usunięta.\n", key);
        return 0;
    } else {
        printf("Pamięć współdzielona z kluczem %d nadal istnieje!\n", key);
        return 1;
    }
}

// Funkcja sprawdzająca semafory (jeśli są używane w systemie)
int sprawdz_semafory(key_t key) {
    int sem_id = semget(key, 0, 0600);
    if (sem_id < 0) {
        printf("Semafory z kluczem %d zostały poprawnie usunięte.\n", key);
        return 0;
    } else {
        printf("Semafory z kluczem %d nadal istnieją!\n", key);
        return 1;
    }
}

int main() {
    int bledy = 0;

    // Sprawdzenie pamięci współdzielonej
    bledy += sprawdz_pamiec_wspoldzielona(SHM_KEY);          // Pamięć kas
    bledy += sprawdz_pamiec_wspoldzielona(SHM_KEY + 1);      // Liczba klientów
    bledy += sprawdz_pamiec_wspoldzielona(SHM_POZAR_KEY);    // Flaga pożaru
    bledy += sprawdz_pamiec_wspoldzielona(SHM_AWARIA_KEY);   // Flaga awarii
    bledy =+ sprawdz_pamiec_wspoldzielona(SHM_PROCESSES_KEY); //Liczba procesów

    // Sprawdzenie semaforów (opcjonalne, jeśli są używane w projekcie)
    bledy += sprawdz_semafory(SEM_KEY);

    if (bledy == 0) {
        printf("Test: Wszystkie zasoby zostały poprawnie zwolnione.\n");
    } else {
        printf("Test: Niektóre zasoby nie zostały poprawnie zwolnione.\n");
    }

    return 0;
}