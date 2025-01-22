#ifndef SUPERMARKET_H
#define SUPERMARKET_H

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

#define MAX_KASY 10           // Maksymalna liczba kas
#define MIN_CZYNNE_KASY 2     // Minimalna liczba otwartych kas
#define KLIENT_PER_KASA 5     // Liczba klientów przypadająca na jedną kasę
#define SHM_KEY 12345         // Klucz do pamięci współdzielonej
#define SEM_KEY 54321         // Klucz do semaforów
#define MAKS_KLIENTOW 1000    // Maksymalna liczba klientów przebywająca jednocześnie  w sklepie - można zmienić do liczby możliwych do wykonania procesów na serwerze
#define SHM_POZAR_KEY 54322 // Klucz do pamięci flagi pożaru
#define SHM_AWARIA_KEY 54323 // Klucz do pamięci flagi awarii prądu
#define SHM_PROCESSES_KEY 0x67890 // Unikalny klucz  przechowującej liczbę aktywnych procesów klientów

typedef struct {
    int kolejka;              // Liczba klientów w kolejce
    int czynna;               // 0 - kasa zamknięta, 1 - kasa otwarta
    int do_zamkniecia;        // 0 - normalna praca, 1 - kasa w stanie zamykania
    int obsluzonych_klientow; // Liczba obsłużonych klientów
} Kasa;

#endif