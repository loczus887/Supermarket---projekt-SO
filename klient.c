#include "supermarket.h"

int main() {
    key_t key = ftok("supermarket", 65);
    int msgid = msgget(key, 0666 | IPC_CREAT);

    srand(time(NULL));
    int id_klienta = 0;

    while (1) {
        Komunikat komunikat;
        komunikat.mtype = MSG_TYPE_KLIENT;
        komunikat.liczba_klientow = 1;
        komunikat.wydatki = rand() % 1000 + 1;

        msgsnd(msgid, &komunikat, sizeof(Komunikat) - sizeof(long), 0);

        printf("Klient %d: Wchodzę do supermarketu.\n", ++id_klienta);

        sleep(rand() % 10 + 1);        // Symulacja czasu zakupów

        komunikat.liczba_klientow = -1; 
        msgsnd(msgid, &komunikat, sizeof(Komunikat) - sizeof(long), 0);

        printf("Klient %d: Opuszczam supermarket.\n", id_klienta);
        
        usleep(rand() % 500000);
    }

    return 0;
}
