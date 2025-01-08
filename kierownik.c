#include "supermarket.h"

extern Kasa kasy[MAX_KASY];
extern int liczba_czynnych_kas;
extern int liczba_klientow;

void *kierownik_zarzadzanie(void *arg) {
    while (1) {
        sleep(1);
        pthread_mutex_lock(&kasy[0].mutex); 
        if (liczba_klientow > liczba_czynnych_kas) {
        }
    
        if (liczba_klientow > liczba_czynnych_kas * KLIENT_PER_KASA && liczba_czynnych_kas < MAX_KASY) {
            liczba_czynnych_kas++;
            printf("Kierownik: Otwieram kasę %d\n", liczba_czynnych_kas);
        }

        if (liczba_czynnych_kas > MIN_CZYNNE_KASY && liczba_klientow < (liczba_czynnych_kas - 1) * KLIENT_PER_KASA) {
            printf("Kierownik: Zamykam kasę %d\n", liczba_czynnych_kas);
            liczba_czynnych_kas--;
        }
        pthread_mutex_unlock(&kasy[0].mutex); 
    }
    return NULL;
}