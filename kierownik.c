#include "supermarket.h"

int main() {
    key_t key = ftok("supermarket", 65);
    int msgid = msgget(key, 0666 | IPC_CREAT);

    int liczba_czynnych_kas = MIN_CZYNNE_KASY;
    int liczba_klientow = 0;

    while (1) {
        Komunikat komunikat;
        msgrcv(msgid, &komunikat, sizeof(Komunikat) - sizeof(long), MSG_TYPE_KLIENT, 0);

        liczba_klientow += komunikat.liczba_klientow;
        printf("Kierownik: Liczba klientów = %d\n", liczba_klientow);

        if (liczba_klientow > liczba_czynnych_kas * KLIENT_PER_KASA && liczba_czynnych_kas < MAX_KASY) {
            liczba_czynnych_kas++;
            printf("Kierownik: Otwieram kasę %d\n", liczba_czynnych_kas);
        }

        if (liczba_czynnych_kas > MIN_CZYNNE_KASY && liczba_klientow < (liczba_czynnych_kas - 1) * KLIENT_PER_KASA) {
            printf("Kierownik: Zamykam kasę %d\n", liczba_czynnych_kas);
            liczba_czynnych_kas--;
        }
    }
    return 0;
}
