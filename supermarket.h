#ifndef SUPERMARKET_H
#define SUPERMARKET_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stddef.h>
#include <string.h>

// Stałe
#define MAX_KASY 10
#define MAX_KLIENCI 10000
#define CZAS_OBSLUGI 1
#define MIN_CZYNNE_KASY 2
#define KLIENT_PER_KASA 5
#define MAX_DNI 7

// Struktura dla kasy
typedef struct {
    int numer_kasy;
    int liczba_klientow;
    int zarobek;
    pthread_mutex_t mutex;
} Kasa;

// Struktura dla klienta
typedef struct {
    int id_klienta;
    int czas_w_sklepie;
    int wydatki;
} Klient;

// Prototypy funkcji
void *kierownik_zarzadzanie(void *arg);
void *klient_zachowanie(void *arg);
void strazak_obsluga(int sig);
void zapis_raportu(int dzien, int zarobki, int klienci, int pozary);
void sigint_obsluga(int sig);

#endif