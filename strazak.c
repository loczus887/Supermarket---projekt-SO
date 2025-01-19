#include "supermarket.h"

void strazak_obsluga(int sig) {
    key_t key = ftok("supermarket", 65);
    int msgid = msgget(key, 0666 | IPC_CREAT);

    printf("Strażak: Pożar! Zamykam supermarket.\n");
    msgctl(msgid, IPC_RMID, NULL);    //Usuwanie kolejki komunikatów
    exit(0);
}

int main() {
    signal(SIGINT, strazak_obsluga);
    while (1) {
        pause();
    }
    return 0;
}
