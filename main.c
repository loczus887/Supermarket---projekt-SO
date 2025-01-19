#include "supermarket.h"
#include <sys/wait.h>

int main() {
    pid_t pid_klient, pid_kierownik, pid_strazak;

    pid_kierownik = fork();         // Proces kierownika
    if (pid_kierownik == 0) {
        execl("./kierownik", "./kierownik", NULL);
        perror("Nie udało się uruchomić procesu kierownika");
        exit(1);
    }

    pid_klient = fork();     // Proces klienta
    if (pid_klient == 0) {
        execl("./klient", "./klient", NULL);
        perror("Nie udało się uruchomić procesu klienta");
        exit(1);
    }

    pid_strazak = fork(); //Proces strażaka
    if (pid_strazak == 0) {
        execl("./strazak", "./strazak", NULL);
        perror("Nie udało się uruchomić procesu strażaka");
        exit(1);
    }

    printf("Wszystkie procesy zostały uruchomione.\n");
    printf("Naciśnij Ctrl+C w oknie strażaka, aby symulować pożar i zakończyć działanie.\n");

    // Oczekiwanie na zakończenie procesów
    waitpid(pid_kierownik, NULL, 0);
    waitpid(pid_klient, NULL, 0);
    waitpid(pid_strazak, NULL, 0);

    printf("Symulacja zakończona.\n");

    return 0;
}
