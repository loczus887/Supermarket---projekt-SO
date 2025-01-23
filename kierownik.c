#include "supermarket.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Flaga do oznaczenia pożaru
volatile sig_atomic_t sygnal_pozar = 0;

// **Funkcja obsługi sygnału pożaru**
void obsluga_pozaru(int sig) {
    sygnal_pozar = 1; // Ustawienie flagi pożaru
}

int main() {
    // **Połączenie z pamięcią współdzieloną dla kas**
    int shm_id = shmget(SHM_KEY, MAX_KASY * sizeof(Kasa), 0600);
    if (shm_id < 0) {
        perror("Nie udało się połączyć z pamięcią współdzieloną");
        exit(1);
    }
    Kasa *kasy = (Kasa *)shmat(shm_id, NULL, 0);

    // **Połączenie z pamięcią współdzieloną dla liczby klientów**
    int shm_klienci_id = shmget(SHM_KEY + 1, sizeof(int), IPC_CREAT | 0600);
    if (shm_klienci_id < 0) {
        perror("Nie udało się połączyć z pamięcią współdzieloną dla liczby klientów");
        exit(1);
    }
    int *liczba_klientow = (int *)shmat(shm_klienci_id, NULL, 0);

    // **Połączenie z pamięcią współdzieloną dla flagi pożaru**
    int shm_pozar_id = shmget(SHM_POZAR_KEY, sizeof(int), 0600);
    if (shm_pozar_id < 0) {
        perror("Nie udało się połączyć z flagą pożaru");
        exit(1);
    }
    int *pozar = (int *)shmat(shm_pozar_id, NULL, 0);

    // **Połączenie z pamięcią współdzieloną dla flagi awarii**
    int shm_awaria_id = shmget(SHM_AWARIA_KEY, sizeof(int), IPC_CREAT | 0600);
    if (shm_awaria_id < 0) {
        perror("Nie udało się połączyć z pamięcią współdzieloną dla flagi awarii");
        exit(1);
    }
    int *awaria = (int *)shmat(shm_awaria_id, NULL, 0);

    // **Obsługa sygnału pożaru**
    signal(SIGINT, obsluga_pozaru);

    int liczba_kas_do_zamkniecia = 0; // Licznik kas oznaczonych do zamknięcia

    // **Główna pętla zarządzania kasami**
    while (1) {
        // **Obsługa pożaru**
        if (*pozar || sygnal_pozar) {
            printf("Kierownik: Pożar! Zamykam wszystkie kasy i ewakuuję klientów.\n");
            for (int i = 0; i < MAX_KASY; i++) {
                if (kasy[i].czynna) {
                    printf("Kierownik: Zamykam kasę %d. Ewakuowani klienci z kolejki %d\n", i + 1, kasy[i].kolejka);
                } else {
                    printf("Kierownik: Kasa %d była już zamknięta.\n", i + 1);
                }
                kasy[i].czynna = 0; // Zamknięcie kasy
            }
            exit(0);
        }

        int czynne_kasy = 0; //Licznik czynnych kas

        // **Liczenie czynnych kas**
        for (int i = 0; i < MAX_KASY; i++) {
            if (kasy[i].czynna) {
                czynne_kasy++;
            }
        }

        printf("Kierownik: Liczba klientów = %d, czynne kasy = %d\n", *liczba_klientow, czynne_kasy);

        // **Otwieranie nowych kas, jeśli jest więcej klientów niż kasy mogą obsłużyć**
        while (*liczba_klientow > KLIENT_PER_KASA * (czynne_kasy - liczba_kas_do_zamkniecia) && czynne_kasy < MAX_KASY) {
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
                break; // Jeśli nie można otworzyć kolejnych kas, przerwij pętlę
            }
        }

        // Oznaczanie kas do zamknięcia, jeśli liczba klientów jest mała
        for (int i = MAX_KASY - 1; i >= 1; i--) {
            if (*liczba_klientow <= KLIENT_PER_KASA * (czynne_kasy - 1 - liczba_kas_do_zamkniecia) && kasy[i].czynna && czynne_kasy > MIN_CZYNNE_KASY && !kasy[i].do_zamkniecia) {
                kasy[i].do_zamkniecia = 1; // Kasa oznaczona do zamknięcia
                liczba_kas_do_zamkniecia++;
                break;
            }
        }

        // **Zamykanie kas, jeśli liczba klientów jest za mała, pod warunkiem, że są puste**
        for (int i = 0; i < MAX_KASY; i++) {
            if (kasy[i].do_zamkniecia && kasy[i].kolejka == 0) {
                kasy[i].czynna = 0; // Zamknięcie kasy
                kasy[i].do_zamkniecia = 0; // Usunięcie oznaczenia do zamknięcia
                czynne_kasy--;
                liczba_kas_do_zamkniecia--;
                printf("Kierownik: Zamykam kasę %d.\n", i + 1);
            }
        }

        usleep(100000); // Oczekiwanie przed kolejną iteracją
    }
}
