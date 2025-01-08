#include "supermarket.h"
#include <string.h>

Kasa kasy[MAX_KASY];
int liczba_czynnych_kas = MIN_CZYNNE_KASY;
int liczba_klientow = 0;
pthread_mutex_t liczba_klientow_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile int przyjmowanie_klientow = 1; 

int dzien = 0, pozary = 0, zarobki = 0;

void zapis_raportu(int dzien, int zarobki, int klienci, int pozary) {
    int fd = open("raport.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("Nie można otworzyć pliku raport.txt");
        return;
    }
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Dzień %d: Zarobki: %d zł, Klienci: %d, Pożary: %d\n", dzien, zarobki, klienci, pozary);
    write(fd, buffer, strlen(buffer));
    close(fd);
}

void *symulacja_dnia(void *arg) {
    for (dzien = 1; dzien <= MAX_DNI; dzien++) { //dopytać ile tych dni ma być
        printf("Dzień %d rozpoczyna się.\n", dzien);
        przyjmowanie_klientow = 1;

        sleep(10); // Czas trwania dnia

        printf("Dzień %d zakończony.\n", dzien);
        przyjmowanie_klientow = 0;

        printf("Koniec dnia %d. Czekam na obsłużenie wszystkich klientów.\n", dzien);
        while (1) {
            pthread_mutex_lock(&liczba_klientow_mutex);
            if (liczba_klientow == 0) {
                pthread_mutex_unlock(&liczba_klientow_mutex);
                break;
            }
            pthread_mutex_unlock(&liczba_klientow_mutex);
            sleep(1); 
        }

        printf("Wszyscy klienci zostali obsłużeni. Zapisuję raport.\n");
        pthread_mutex_lock(&liczba_klientow_mutex);
        int klienci_do_raportu = liczba_klientow;
        pthread_mutex_unlock(&liczba_klientow_mutex);
        zapis_raportu(dzien, zarobki, klienci_do_raportu, pozary);

        // Reset danych po zapisaniu raportu, problem-> resetuje się liczba klientów przez używanie jej do kontroli(notatnik)
        zarobki = 0;
        pozary = 0;
    }
    pthread_exit(NULL);
}



int main() {
    srand(time(NULL));

    pthread_t kierownik_thread;
    pthread_t dzien_thread;

    for (int i = 0; i < MAX_KASY; i++) {
        kasy[i].numer_kasy = i + 1;
        kasy[i].liczba_klientow = 0;
        kasy[i].zarobek = 0;
        pthread_mutex_init(&kasy[i].mutex, NULL);
    }

    // Ustawienie obsługi sygnału pożaru, coś nie działa dobrze jeszcze
    struct sigaction sa;
    sa.sa_handler = strazak_obsluga;
    sigaction(SIGUSR1, &sa, NULL);

    // Uruchomienie wątku kierownika i symulacji dnia
    pthread_create(&kierownik_thread, NULL, kierownik_zarzadzanie, NULL);
    pthread_create(&dzien_thread, NULL, symulacja_dnia, NULL);

    // Tworzenie klientów
    int id_klienta = 0;
    while (dzien <= MAX_DNI) {
        if (!przyjmowanie_klientow) {
        sleep(1); // Oczekiwanie, aż przyjmowanie klientów zostanie wznowione
        continue;
    }

    Klient *klient = malloc(sizeof(Klient));
    klient->id_klienta = ++id_klienta;
    klient->czas_w_sklepie = rand() % 10 + 1;
    klient->wydatki = rand() % 1000 + 1;

    pthread_t klient_thread;
    pthread_create(&klient_thread, NULL, klient_zachowanie, klient);
    pthread_detach(klient_thread); // Umożliwia automatyczne zwolnienie zasobów po zakończeniu wątku klienta

    sleep(rand() % 2); // Klienci przychodzą losowo
}

    pthread_join(dzien_thread, NULL);
    pthread_cancel(kierownik_thread);

    for (int i = 0; i < MAX_KASY; i++) {
        pthread_mutex_destroy(&kasy[i].mutex);
    }

    return 0;
}