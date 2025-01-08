#include "supermarket.h"

extern Kasa kasy[MAX_KASY];
extern int liczba_czynnych_kas;

void strazak_obsluga(int sig) {
    printf("Pożar! Klienci opuszczają supermarket.\n");

    for (int i = 0; i < liczba_czynnych_kas; i++) {
        pthread_mutex_lock(&kasy[i].mutex);
        kasy[i].liczba_klientow = 0;
        pthread_mutex_unlock(&kasy[i].mutex);
    }

    liczba_czynnych_kas = 0;
    exit(0);
}