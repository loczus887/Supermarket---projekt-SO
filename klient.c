#include "supermarket.h"

extern Kasa kasy[MAX_KASY];
extern pthread_mutex_t liczba_klientow_mutex;
extern int liczba_klientow;
extern int liczba_czynnych_kas; 

void *klient_zachowanie(void *arg) {
    Klient *klient = (Klient *)arg;

    pthread_mutex_lock(&liczba_klientow_mutex);
    liczba_klientow++;
    pthread_mutex_unlock(&liczba_klientow_mutex);

    printf("Klient %d: Wchodzę do supermarketu.\n", klient->id_klienta);
    sleep(klient->czas_w_sklepie);

    int wybrana_kasa = -1, min_klientow = MAX_KLIENCI;
    for (int i = 0; i < liczba_czynnych_kas; i++) {
        pthread_mutex_lock(&kasy[i].mutex);
        if (kasy[i].liczba_klientow < min_klientow) {
            min_klientow = kasy[i].liczba_klientow;
            wybrana_kasa = i;
        }
        pthread_mutex_unlock(&kasy[i].mutex);
    }

    pthread_mutex_lock(&kasy[wybrana_kasa].mutex);
    kasy[wybrana_kasa].liczba_klientow++;
    pthread_mutex_unlock(&kasy[wybrana_kasa].mutex);

    printf("Klient %d: Czekam w kolejce do kasy %d.\n", klient->id_klienta, wybrana_kasa + 1);
    sleep(CZAS_OBSLUGI);

    pthread_mutex_lock(&kasy[wybrana_kasa].mutex);
    kasy[wybrana_kasa].liczba_klientow--;
    kasy[wybrana_kasa].zarobek += klient->wydatki;
    pthread_mutex_unlock(&kasy[wybrana_kasa].mutex);

    pthread_mutex_lock(&liczba_klientow_mutex);
    liczba_klientow--;
    pthread_mutex_unlock(&liczba_klientow_mutex);

    printf("Klient %d: Zostałem obsłużony i wychodzę.\n", klient->id_klienta);
    free(klient);
    return NULL;
}
