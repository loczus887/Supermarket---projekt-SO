#ifndef SUPERMARKET_H
#define SUPERMARKET_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
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

typedef struct {
    int kolejka;              // Liczba klientów w kolejce
    int czynna;               // 0 - kasa zamknięta, 1 - kasa otwarta
} Kasa;

#endif