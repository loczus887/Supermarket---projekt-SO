#include "supermarket.h"
#include <termios.h>
#include <unistd.h>

// Deklaracje zmiennych globalnych
Kasa kasy[MAX_KASY];
int liczba_czynnych_kas = MIN_CZYNNE_KASY;
int liczba_klientow = 0;
pthread_mutex_t liczba_klientow_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile int przyjmowanie_klientow = 1;

int dzien = 0, zarobki = 0;

// Funkcja do ustawienia trybu niekanonicznego
void ustaw_tryb_niekanoniczny() {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~(ICANON | ECHO); // Wyłącza tryb kanoniczny i echo
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

// Funkcja do przywrócenia trybu kanonicznego
void przywroc_tryb_kanoniczny() {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag |= (ICANON | ECHO); // Włącza tryb kanoniczny i echo
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

// Funkcja nasłuchująca klawiaturę
void *nasluch_klawiatury(void *arg) {
    char c;
    while (1) {
        c = getchar(); // Oczekuje na wpisanie znaku
        if (c == 's') {
            strazak_obsluga(0); // Wywołuje funkcję obsługi pożaru
        }
    }
    return NULL;
}

// Funkcja zapisująca raport
void zapis_raportu(int dzien, Kasa kasy[], int liczba_kas) {
    int fd = open("raport.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("Nie można otworzyć pliku raport.txt");
        return;
    }

    char buffer[512];
    int total_klienci = 0, total_zarobki = 0;

    snprintf(buffer, sizeof(buffer), "Dzień %d:\n", dzien);
    write(fd, buffer, strlen(buffer));

    for (int i = 0; i < liczba_kas; i++) {
        pthread_mutex_lock(&kasy[i].mutex);
        snprintf(buffer, sizeof(buffer),
                 "Kasa %d: Klienci: %d, Zarobki: %d zł\n",
                 kasy[i].numer_kasy, kasy[i].liczba_klientow, kasy[i].zarobek);
        total_klienci += kasy[i].liczba_klientow;
        total_zarobki += kasy[i].zarobek;
        pthread_mutex_unlock(&kasy[i].mutex);
        write(fd, buffer, strlen(buffer));
    }

    snprintf(buffer, sizeof(buffer),
             "Podsumowanie dnia %d: Klienci: %d, Zarobki: %d zł\n\n",
             dzien, total_klienci, total_zarobki);
    write(fd, buffer, strlen(buffer));

    close(fd);
}

// Funkcja obsługi SIGINT
void sigint_obsluga(int sig) {
    printf("Przerwanie programu. Zapisuję końcowy raport.\n");

    pthread_mutex_lock(&liczba_klientow_mutex);
    int klienci_do_raportu = liczba_klientow;
    pthread_mutex_unlock(&liczba_klientow_mutex);

    zapis_raportu(dzien, kasy, MAX_KASY);
    przywroc_tryb_kanoniczny(); // Przywrócenie ustawień terminala
    exit(0);
}

// Funkcja symulacji dnia
void *symulacja_dnia(void *arg) {
    for (dzien = 1; dzien <= MAX_DNI; dzien++) {
        printf("Dzień %d rozpoczyna się.\n", dzien);
         liczba_czynnych_kas = MIN_CZYNNE_KASY;
        przyjmowanie_klientow = 1;

        sleep(1); // Czas trwania dnia

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
        liczba_czynnych_kas = 0;

        printf("Wszyscy klienci zostali obsłużeni. Zapisuję raport.\n");
        zapis_raportu(dzien, kasy, MAX_KASY);

        zarobki = 0;
        sleep(1);
    }
    pthread_exit(NULL);
}

int main() {
    srand(time(NULL));

    pthread_t kierownik_thread;
    pthread_t dzien_thread;
    pthread_t klawiatura_thread; // Wątek nasłuchujący klawiatury

    // Inicjalizacja kas
    for (int i = 0; i < MAX_KASY; i++) {
        kasy[i].numer_kasy = i + 1;
        kasy[i].liczba_klientow = 0;
        kasy[i].zarobek = 0;
        pthread_mutex_init(&kasy[i].mutex, NULL);
    }

    // Ustawienie trybu niekanonicznego
    ustaw_tryb_niekanoniczny();

    // Ustawienie obsługi sygnału SIGINT
    struct sigaction sa;
    sa.sa_handler = sigint_obsluga;
    sigaction(SIGINT, &sa, NULL);

    // Uruchomienie wątków
    pthread_create(&kierownik_thread, NULL, kierownik_zarzadzanie, NULL);
    pthread_create(&dzien_thread, NULL, symulacja_dnia, NULL);
    pthread_create(&klawiatura_thread, NULL, nasluch_klawiatury, NULL);

    // Tworzenie klientów
    int id_klienta = 0;
    while (dzien <= MAX_DNI) {
        if (!przyjmowanie_klientow) {
            sleep(1);
            continue;
        }

        Klient *klient = malloc(sizeof(Klient));
        if (klient == NULL) {
    fprintf(stderr, "Błąd alokacji pamięci dla klienta\n");
    exit(1);
}
        klient->id_klienta = ++id_klienta;
        klient->czas_w_sklepie = rand() % 10 + 1;
        klient->wydatki = rand() % 1000 + 1;

        pthread_t klient_thread;
        pthread_create(&klient_thread, NULL, klient_zachowanie, klient);
        pthread_detach(klient_thread);

        usleep(rand() % 9000);  // losowe opóźnienie między 0 a 9000 mikrosekund
    }

    // Czekanie na zakończenie wątku dnia
    pthread_join(dzien_thread, NULL);
    pthread_cancel(kierownik_thread);
    pthread_cancel(klawiatura_thread);

    // Przywrócenie trybu kanonicznego
    przywroc_tryb_kanoniczny();

    // Zwalnianie zasobów
    for (int i = 0; i < MAX_KASY; i++) {
        pthread_mutex_destroy(&kasy[i].mutex);
    }

    return 0;
}