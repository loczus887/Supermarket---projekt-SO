#include "supermarket.h"
#include <signal.h>

// Flaga do oznaczenia pożaru
volatile sig_atomic_t sygnal_pozar = 0;

// Funkcja obsługi sygnału pożaru
void obsluga_pozaru(int sig) {
    sygnal_pozar = 1;
}

int main() {
    // Połączenie z pamięcią współdzieloną
    int shm_id = shmget(SHM_KEY, MAX_KASY * sizeof(Kasa), 0666);
    if (shm_id < 0) {
        perror("Nie udało się połączyć z pamięcią współdzieloną");
        exit(1);
    }

    Kasa *kasy = (Kasa *)shmat(shm_id, NULL, 0);

    // Połączenie do zmiennej przechowującej liczbę klientów w sklepie
    int shm_klienci_id = shmget(SHM_KEY + 1, sizeof(int), IPC_CREAT | 0666);
    if (shm_klienci_id < 0) {
        perror("Nie udało się utworzyć pamięci dla liczby klientów w sklepie");
        exit(1);
    }
    int *liczba_klientow = (int *)shmat(shm_klienci_id, NULL, 0);
    *liczba_klientow = 0;

    // Połączenie do flagi pożaru
    int shm_pozar_id = shmget(SHM_POZAR_KEY, sizeof(int), 0666);
    if (shm_pozar_id < 0) {
        perror("Nie udało się połączyć z pamięcią flagi pożaru");
        exit(1);
    }
    int *pozar = (int *)shmat(shm_pozar_id, NULL, 0);

    // Połączenie do pamięci flagi awarii
    int shm_awaria_id = shmget(SHM_AWARIA_KEY, sizeof(int), IPC_CREAT | 0666);
    if (shm_awaria_id < 0) {
        perror("Nie udało się utworzyć pamięci flagi awarii");
        exit(1);
    }
    int *awaria = (int *)shmat(shm_awaria_id, NULL, 0);
    *awaria = 0; // Początkowa wartość - brak awarii

    // Ustawienie obsługi sygnału pożaru
    signal(SIGINT, obsluga_pozaru);
    int liczba_kas_do_zamkniecia = 0;
    while (1) {
        if (*pozar || sygnal_pozar) {
            for (int i = 0; i < MAX_KASY; i++) {
                if (kasy[i].czynna) {
                    printf("Kierownik: Zamykam kasę %d. Ewakuowani klienci z kolejki %d\n", i + 1, kasy[i].kolejka);
                } else {
                    printf("Kierownik: Kasa %d była już zamknięta.\n", i + 1);
                }
                kasy[i].czynna = 0;
            }

            exit(0);
        }

        int czynne_kasy = 0;


        // Liczenie czynnych kas
        for (int i = 0; i < MAX_KASY; i++) {
            if (kasy[i].czynna) {
                czynne_kasy++;
            }
        }

        printf("Kierownik: liczba_klientow = %d, czynne_kasy = %d\n", *liczba_klientow, czynne_kasy);

        // Otwieranie nowych kas lub przywracanie oznaczonych do zamknięcia
    while (*liczba_klientow > KLIENT_PER_KASA * (czynne_kasy-liczba_kas_do_zamkniecia) && czynne_kasy < MAX_KASY) {
        int otwarto = 0;
        for (int i = 0; i < MAX_KASY; i++) {
            // Przywracanie kas oznaczonych do zamknięcia
            if (kasy[i].do_zamkniecia) {
                kasy[i].do_zamkniecia = 0;
                liczba_kas_do_zamkniecia--;
                czynne_kasy++;
                printf("Kierownik: Przywracam do działania kasę %d.\n", i + 1);
                otwarto = 1;
                break;
            }
            // Otwieranie nowych kas
            else if (!kasy[i].czynna && !kasy[i].do_zamkniecia) {
                kasy[i].czynna = 1;
                czynne_kasy++;
                printf("Kierownik: Otwieram kasę %d.\n", i + 1);
                otwarto = 1;
                break;
            }
        }
        if (!otwarto) {
            break;
        }
    }

        // Oznaczanie kas do zamknięcia
        for (int i = MAX_KASY - 1; i >= 0; i--) {
            if (*liczba_klientow <= KLIENT_PER_KASA * (czynne_kasy-1-liczba_kas_do_zamkniecia) && kasy[i].czynna && czynne_kasy > MIN_CZYNNE_KASY && !kasy[i].do_zamkniecia) {
                kasy[i].do_zamkniecia = 1;
                liczba_kas_do_zamkniecia++;
                break;
            }
        }

        // Zamykamy kasy oznaczone do zamknięcia, które są puste
        for (int i = 0; i < MAX_KASY; i++) {
            if (kasy[i].do_zamkniecia && kasy[i].kolejka == 0) {
                kasy[i].czynna = 0;
                kasy[i].do_zamkniecia = 0;
                czynne_kasy--;
                liczba_kas_do_zamkniecia--;
                printf("Kierownik: Zamykam kasę %d.\n", i + 1);
            }
        }

        usleep(100000);
    }
}