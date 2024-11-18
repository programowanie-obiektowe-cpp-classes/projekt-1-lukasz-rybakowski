#include <iostream>
#include <algorithm>
#include <memory>
#include <random>
#include <string>
#include <vector>



const int    CI         = 100;
const int    CMag       = 50;
const int    CMkt       = 80;
const int    CR         = 120;
const int    MAX_KREDYT = 36;
const double M          = 1.5;

std::default_random_engine               generator;
std::uniform_real_distribution< double > reklamacje_dist(0.0, 0.02);
std::uniform_real_distribution< double > uszkodzenia_dist(0.0, 0.01);
std::uniform_real_distribution< double > cena_popyt_dist(0.9, 1.1);

int next_inz_id = 1;
int next_mag_id = 1;
int next_mkt_id = 1;
int next_rob_id = 1;


class Pracownik
{
public:
    Pracownik(std::string imie) : imie(imie) {}
    virtual ~Pracownik() = default;
    virtual void print() const = 0;

protected:
    std::string imie;

};

class Inz : public Pracownik
{
    public:
        Inz(std::string imie, std::string wydzial) : Pracownik(imie), wydzial(wydzial) {}
        void print() const override {std::cout << "Inzynier:" << imie << ", Wydzial:" << wydzial << "\n"; }

    private:
        std::string wydzial;
};

class Mag : public Pracownik
{
    public:
        Mag(std::string imie, bool obsl_widl) : Pracownik(imie), obsl_widl(obsl_widl) {}
        void print() const override {std::cout << "Magazynier:" << imie << ", Obsluga wozka widlowego:" << (obsl_widl ? "tak" : "nie") << "\n";}
    private:
        bool obsl_widl;
};

class Mkt : public Pracownik
{
    public :
        Mkt(std::string imie, int obserwujacy) : Pracownik(imie), obserwujacy(obserwujacy) {}
        void print() const override {std::cout << "Marketer:" << imie << ", Liczba obserwujacych" << obserwujacy << "\n";}

    private:
        int obserwujacy;
};

class Rob : public Pracownik
{
    public:
        Rob(std::string imie, double but) : Pracownik(imie), but(but) {}
        void print() const override {std::cout << "Robotnik:" << imie << ", Rozmiar buta:" << but << "\n";}
    private:
        double but;
};

class Kredyt
{
    private:
        double dlug;
        int pozostale_raty;
    public:
        Kredyt(double kwota, int pozostale_raty) : pozostale_raty(pozostale_raty) {
            double oprocentowanie = 0.05;
            double calkowity_dlug = kwota * (1 + (oprocentowanie * (pozostale_raty/12.0)));
            dlug = calkowity_dlug;
        }
        double splac_rate() {
            if (pozostale_raty > 0) {
                double rata = dlug / pozostale_raty;
                pozostale_raty --;
                return rata;
            }
            return 0.0;
        }
        bool czy_splacony() const {
            return pozostale_raty == 0;
        }
        int get_pozostale_raty() const { return pozostale_raty; }

};

class Firma 
{
    private:
        double                                      stan_konta;
        int                                         n_prac; // liczba pracowników
        int                                         n_kredytow;
        int                                         n_mag;
        int                                         n_inz;
        int                                         n_mkt;
        int                                         n_rob;
        int                                         magazyn;
        std::vector< std::unique_ptr< Pracownik > > prac;
        std::vector< std::unique_ptr< Kredyt > >    kredyty;
        std::vector<double>                          ostatnie_przychody;

    public:
        Firma() : stan_konta(10000), n_prac(0), n_kredytow(0), n_mag(0), n_inz(0), n_mkt(0), n_rob(0), magazyn(0)
    {
        zatrudnij(std::make_unique< Inz >("Inz_" + std::to_string(next_inz_id++), "Mechanika"));
        zatrudnij(std::make_unique< Mag >("Mag_" + std::to_string(next_mag_id++), true));
        zatrudnij(std::make_unique< Mkt >("Mkt_" + std::to_string(next_mkt_id++), 1000));
        zatrudnij(std::make_unique< Rob >("Rob_" + std::to_string(next_rob_id++), 42.5));
    }

    void zatrudnij(std::unique_ptr< Pracownik > pracownik)
    {
        if (dynamic_cast< Inz* >(pracownik.get()))
        {
            n_inz++;
        }
        else if (dynamic_cast< Mag* >(pracownik.get()))
        {
            n_mag++;
        }
        else if (dynamic_cast< Mkt* >(pracownik.get()))
        {
            n_mkt++;
        }
        else if (dynamic_cast< Rob* >(pracownik.get()))
        {
            n_rob++;
        }
        prac.push_back(std::move(pracownik));
        n_prac++;
    }

    void wez_kredyt(double kwota, int czas_splaty)
    {
        if (czas_splaty > MAX_KREDYT) {
            std::cout << "Zbyt dlugi okres splaty. Maksymalny okres: " << MAX_KREDYT << " miesiecy.";
            return;
        }

        double wartosc_spolki = oblicz_wartosc_spolki();
        double maksymalne_zadluzenie = M * wartosc_spolki;
        double obecne_zadluzenie = 0;
        for (const auto& kredyt : kredyty) {
            if (!kredyt->czy_splacony()) {
                obecne_zadluzenie += kredyt->splac_rate() * kredyt->get_pozostale_raty();
            }
        }
        if (obecne_zadluzenie + kwota > maksymalne_zadluzenie) {
            std::cout << "Zadluzenie przekracza maksymalny dopuszczalny limit! Nie mozna zaciagnac kredytu.";
            return;
        }
        kredyty.push_back(std::make_unique< Kredyt >(kwota, czas_splaty));
        n_kredytow++;
        stan_konta += kwota;
    }

    void zaplac_wynagrodzenie()
    {
        // Wynagrodzenie dla poszczegolnych rodzajow pracownikow
        int wynagrodzenie_inz = n_inz * 3000;
        int wynagrodzenie_mag = n_mag * 2500;
        int wynagrodzenie_mkt = n_mkt * 2000;
        int wynagrodzenie_rob = n_rob * 1500;

        int wynagrodzenie = wynagrodzenie_inz + wynagrodzenie_mag + wynagrodzenie_mkt + wynagrodzenie_rob;
        stan_konta -= wynagrodzenie;
    }
    
    void splac_raty() {
        for (auto it = kredyty.begin(); it != kredyty.end(); ) {
            double rata = (*it)->splac_rate();
            stan_konta -= rata;
            if ((*it)->czy_splacony()) {
                it = kredyty.erase(it);
                n_kredytow--;
            } else {
                ++it;
            }
        }
    }
    static const int N;
    double oblicz_przychod()
    {
        int pojemnosc_magazynu = n_mag * CMag;
        int cena_produktu      = n_inz * CI;
        int popyt              = n_mkt * CMkt;

        // Uwzglednienie wahan cen produktu oraz popytu
        cena_produktu *= cena_popyt_dist(generator);
        popyt *= cena_popyt_dist(generator);

        int teoretyczna_produkcja = n_rob * CR;

        // Uwzglednienie uszkodzen w trakcie produkcji
        int uszkodzenia = static_cast< int >(teoretyczna_produkcja * uszkodzenia_dist(generator));
        teoretyczna_produkcja -= uszkodzenia;

        int faktyczna_produkcja = std::min(teoretyczna_produkcja, pojemnosc_magazynu - magazyn);

        int dostepne_produkty = magazyn + faktyczna_produkcja;
        int sprzedane         = std::min(dostepne_produkty, popyt);

        // Uwzglednienie reklamacji
        int reklamacje = static_cast< int >(sprzedane * reklamacje_dist(generator));

        magazyn = dostepne_produkty - sprzedane;

        return (sprzedane - reklamacje) * cena_produktu;
    }

    void otrzymaj_przychod() {
        double przychod = oblicz_przychod();
        aktualizuj_przychody(przychod);
        stan_konta += przychod; }

    double oblicz_wartosc_spolki() const {
        int liczba_miesiecy = std::min(static_cast<int>(ostatnie_przychody.size()), N);
        if (liczba_miesiecy == 0) return 0.0;
        double suma_przychodow = 0;
        for (int i = 0; i < liczba_miesiecy; ++i) {
            suma_przychodow += ostatnie_przychody[i];
        }
        return suma_przychodow / liczba_miesiecy;
    }
    
    void aktualizuj_przychody(double przychod) {
        if (ostatnie_przychody.size() >= N) {
            ostatnie_przychody.erase(ostatnie_przychody.begin());
        }
        ostatnie_przychody.push_back(przychod);
    }

    void drukuj_pracownikow() const
    {
        std::cout << "Podsumowanie pracownikow:\n";
        std::cout << "Inzynierowie: " << n_inz << "\n";
        std::cout << "Magazynierowie: " << n_mag << "\n";
        std::cout << "Marketerzy: " << n_mkt << "\n";
        std::cout << "Robotnicy: " << n_rob << "\n";
        for (const auto& pracownik : prac)
        {
            pracownik->print();
        }
    }

    void drukuj_informacje() const
    {
        int cena_produktu         = n_inz * CI;
        int popyt                 = n_mkt * CMkt;
        int teoretyczna_produkcja = n_rob * CR;
        int pojemnosc_magazynu    = n_mag * CMag;

        std::cout << "Informacje o firmie: \n";
        std::cout << "Cena produktu: " << cena_produktu << "\n";
        std::cout << "Popyt: " << popyt << "\n";
        std::cout << "Teoretyczna produkcja: " << teoretyczna_produkcja << "\n";
        std::cout << "Pojemnosc magazynu: " << pojemnosc_magazynu << "\n";
        std::cout << "Magazyn: " << magazyn << "\n";
        std::cout << "Wartosc spolki: " << oblicz_wartosc_spolki() << "\n";
    }

    double get_stan_konta() const { return stan_konta; }

};

class Gra
{
public:
    Gra() : firma(std::make_unique< Firma >()), stan(true) {}

    void akcja_gracza()
    {
        std::string komenda;
        std::cout << "Wpisz komende: ";
        std::cin >> komenda;

        if (komenda == "lp")
        {
            firma->drukuj_pracownikow();
        }
        else if (komenda == "zinz")
        {
            std::string imie, nazwisko, wydzial;
            std::cout << "Podaj imie inzyniera: ";
            std::cin >> imie;
            std::cout << "Podaj nazwisko inzyniera: ";
            std::cin >> nazwisko;
            std::cout << "Podaj nazwe wydzialu: ";
            std::cin >> wydzial;
            firma->zatrudnij(std::make_unique< Inz >("Inz_" + std::to_string(next_inz_id++) + " " + imie + " " + nazwisko, wydzial));
        }
        else if (komenda == "zmag")
        {
            std::string imie, nazwisko;
            char obsluga_widl;
            bool obsl_widl;
            std::cout << "Podaj imie magazyniera: ";
            std::cin >> imie;
            std::cout << "Podaj nazwisko magazyniera: ";
            std::cin >> nazwisko;
            std::cout << "Czy magazynier obsluguje wozek widlowy? (t/n): ";
            std::cin >> obsluga_widl;
            obsl_widl = (obsluga_widl == 't' || obsluga_widl == 'T');
            firma->zatrudnij(std::make_unique< Mag >("Mag_" + std::to_string(next_mag_id++) + " " + imie + " " + nazwisko, obsl_widl));
        }
        else if (komenda == "zmkt")
        {
            std::string imie, nazwisko;
            int followers;
            std::cout << "Podaj imie marketera: ";
            std::cin >> imie;
            std::cout << "Podaj nazwisko marketera: ";
            std::cin >> nazwisko;
            std::cout << "Podaj liczbe obserwujacych na mediach spolecznosciowych: ";
            std::cin >> followers;
            firma->zatrudnij(std::make_unique< Mkt >("Mkt_" + std::to_string(next_mkt_id++) + " " + imie + " " + nazwisko, followers));
        }
        else if (komenda == "zrob")
        {
            std::string imie, nazwisko;
            double rozmiar_buta;
            std::cout << "Podaj imie robotnika: ";
            std::cin >> imie;
            std::cout << "Podaj nazwisko robotnika: ";
            std::cin >> nazwisko;
            std::cout << "Podaj rozmiar buta robotnika: ";
            std::cin >> rozmiar_buta;
            firma->zatrudnij(std::make_unique< Rob >("Rob_" + std::to_string(next_rob_id++) + " " + imie + " " + nazwisko, rozmiar_buta));
        }
        else if (komenda == "kred")
        {
            double kwota;
            int    czas;
            std::cout << "Podaj kwote kredytu: ";
            std::cin >> kwota;
            std::cout << "Podaj czas splaty w miesiacach: ";
            std::cin >> czas;
            firma->wez_kredyt(kwota, czas);
        }
        else if (komenda == "kt")
        {
            firma->otrzymaj_przychod();
            firma->zaplac_wynagrodzenie();
            firma->splac_raty();
            std::cout << "Stan konta: " << firma->get_stan_konta() << "\n";
        }
        else if (komenda == "info")
        {
            firma->drukuj_informacje();
        }
        else
        {
            std::cout << "Nieznana komenda." << std::endl;
        }
    }

    bool get_stan() const { return stan; }

    void tick()
    {
        if (firma->get_stan_konta() < 0)
        {
            std::cout << "Firma zbankrutowala!\n";
            stan = false;
        }
        else if (firma->oblicz_wartosc_spolki() > 100000) // Zakładamy, że wartość spółki powyżej 100000 oznacza zwycięstwo
        {
            std::cout << "Gratulacje! Osiagnales wymagana wartosc spolki i wygrales gre!\n";
            stan = false;
        }
    }

private:
    std::unique_ptr< Firma > firma;
    bool                     stan;
};

   
const int Firma::N = 3;

int main()
{
    std::cout << "Dostepne komendy: \n";
    std::cout << "lp - drukuj pracownikow \n";
    std::cout << "zinz - zatrudnij inzyniera \n";
    std::cout << "zmag - zatrdunij magistra \n";
    std::cout << "zmkt - zatrudnij marketera \n";
    std::cout << "zrob - ztrudnij robotnika \n";
    std::cout << "kred - wez kredyt \n";
    std::cout << "kt - wydrukuj stan konta po odliczeniu wszystkich kosztow\n";
    std::cout << "info - wydrukuj informacje o firmie \n";


    Gra gra;
    while (gra.get_stan())
    {
        gra.akcja_gracza();
        gra.tick();
    }
    return 0;
}
