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
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

#define MAX_KASY 10           //Maksymalna liczba kas
#define MIN_CZYNNE_KASY 2     //Minimalna liczba otwartych kas
#define KLIENT_PER_KASA 5     //Liczba klientów przypadająca na jedną kasę
#define MSG_SZ 256            //Rozmiar komunikatu
#define MSG_TYPE_KLIENT 1     //Typ komunikatu dla klientów
#define MSG_TYPE_KIEROWNIK 2  //Typ komunikatu dla kierownika

typedef struct {
    long mtype;               //Typ komunikatu (1 dla klientów, 2 dla kierownika)
    int liczba_klientow;      //Zmiana liczby klientów (dla kierownika)
    int kasa;                 //Numer wybranej kasy (dla klienta)
    int wydatki;              //Wydatki klienta (dla kierownika)
} Komunikat;

#endif
