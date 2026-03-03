#ifndef GAMELIB_H
#define GAMELIB_H
#define ROSSO  "\x1B[31m"
#define VERDE  "\x1B[32m"
#define GIALLO  "\e[1;93m"
#define CIANO  "\e[1;96m"
#define MAGENTA "\e[1;95m"
#define BLU "\e[1;94m"
#define STOCK "\e[0m"

struct Zona_soprasotto; 
struct Zona_mondoreale;

typedef enum {
    bosco, 
    scuola, 
    laboratorio, 
    caverna, 
    strada, 
    giardino, 
    supermercato, 
    centrale_elettrica, 
    deposito_abbandonato, 
    stazione_polizia
} Tipo_zona;

typedef enum {
    nessun_nemico, 
    billi, 
    democane, 
    demotorzone
} Tipo_nemico;

typedef enum { 
    nessun_oggetto, 
    bicicletta, 
    maglietta_fuocoinferno, 
    bussola, 
    schitarrata_metallica
} Tipo_oggetto;

struct Zona_mondoreale {
    Tipo_zona tipo; 
    Tipo_nemico nemico;
    Tipo_oggetto oggetto; 
    struct Zona_mondoreale *avanti;
    struct Zona_mondoreale *indietro;
    struct Zona_soprasotto *link_soprasotto; // Puntatore all'altra dimensione

};

struct Zona_soprasotto {
    Tipo_zona tipo; 
    Tipo_nemico nemico;

    struct Zona_soprasotto *avanti;
    struct Zona_soprasotto *indietro;
    struct Zona_mondoreale *link_mondoreale; 

};

struct Giocatore {
    char nome[50]; 
    int mondo; // 0 = Reale, 1 = Soprasotto
    struct Zona_mondoreale *pos_mondoreale;
    struct Zona_soprasotto *pos_soprasotto;
    Tipo_oggetto zaino[3];
    int attacco_psichico;
    int difesa_psichica;
    int fortuna;
    int hp;
    int color;
    int vivo;
    int boost_critico;
};

void imposta_gioco();
void gioca();
void termina_gioco();
void crediti();
void regole();
char getch(void);
int gestisci_navigazione(int *scelta, int max_scelta);
void stampa_menu(const char* opzioni[], int n_opzioni, int scelta);

#endif