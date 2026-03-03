#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include "gamelib.h"
#include <string.h>
#include <ctype.h>
#include <time.h>


// VARIABILI GLOBALI //
static int numGiocatori = 0;
static int gioco_impostato = 0;
static struct Giocatore* player[4] = {NULL, NULL, NULL, NULL};
static char* playerColor[4] = {GIALLO, CIANO, MAGENTA, BLU};
static char* allColors[20] = {
    "\033[38;5;196m", // Rosso
    "\033[38;5;202m", // Arancio Rossastro
    "\033[38;5;208m", // Arancio
    "\033[38;5;214m", // Giallo Oro
    "\033[38;5;220m", // Oro
    "\033[38;5;226m", // Giallo
    "\033[38;5;190m", // Giallo Lime
    "\033[38;5;118m", // Lime
    "\033[38;5;46m",  // Verde Acceso
    "\033[38;5;48m",  // Verde Mare
    "\033[38;5;49m",  // Turchese
    "\033[38;5;51m",  // Ciano
    "\033[38;5;45m",  // Azzurro
    "\033[38;5;39m",  // Blu Cielo
    "\033[38;5;33m",  // Blu
    "\033[38;5;27m",  // Blu Profondo
    "\033[38;5;57m",  // Indaco
    "\033[38;5;93m",  // Viola
    "\033[38;5;129m", // Magenta
    "\033[38;5;201m"  // Rosa Shocking
};
static struct Zona_mondoreale* prima_zona_mondoreale = NULL;
static struct Zona_soprasotto* prima_zona_soprasotto = NULL;
static char storicoVincitori[3][50] = {"", "", ""};
static int nPlayerMorti;
static int statsModificate[4];
static int undicivirgolacinqueUsato = 0;
static int mappaGenerata = 0;
static int nessunMorto = 1;
static int demotorzoneSconfitto = 0;





// DICHIARAZIONE FUN STATICHE LOCALI //
static const char* nome_zona(Tipo_zona z);
static const char* nome_nemico(Tipo_nemico n);
static const char* nome_oggetto(Tipo_oggetto o);
static void modificaStats(int i);
static int generaMappa();
static void creazioneMappa();
static void cancella_mappa_esistente();
static int inserisciZona();
static void cancellaZona();
static void stampaMappa();
static void stampaZona();
static void giocaTurno(int i);
static int combatti(struct Giocatore* p, Tipo_nemico nemicoQui);
static int apriZaino(struct Giocatore* p);



// FUNZIONI UTILI //
char getch(void) {
    char buf = 0;
    struct termios old = {0}, newt = {0};
    tcgetattr(0, &old);
    newt = old;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &newt);
    read(0, &buf, 1);
    tcsetattr(0, TCSANOW, &old);
    return buf;
}

static int getchNoWait(void) {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    newt.c_cc[VMIN] = 0; // Non blocca in attesa di input
    newt.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    char c;
    int bytesRead = read(STDIN_FILENO, &c, 1);
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    if(bytesRead > 0) return 1; // Tasto premuto
    return 0; // Nessun tasto
}

static void cancella_mappa_esistente() {
    struct Zona_mondoreale* temp_mr = prima_zona_mondoreale;
    while (temp_mr != NULL) {
        struct Zona_mondoreale* prossimo = temp_mr->avanti;
        free(temp_mr);
        temp_mr = prossimo;
    }
    prima_zona_mondoreale = NULL;

    struct Zona_soprasotto* temp_ss = prima_zona_soprasotto;
    while (temp_ss != NULL) {
        struct Zona_soprasotto* prossimo = temp_ss->avanti;
        free(temp_ss);
        temp_ss = prossimo;
    }
    prima_zona_soprasotto = NULL;
}

static Tipo_nemico random_nemico_mondoreale() {
    int r = rand() % 100;
    if(r < 40) return nessun_nemico;
    if(r > 40 && r < 70) return democane;
    else return billi;
}

static Tipo_nemico random_nemico_soprasotto() {
    int r = rand() % 100;
    if(r < 40) return nessun_nemico;
    else return democane;
}

static Tipo_oggetto random_oggetto() {
    int r = rand() % 100;
    if (r < 18) return nessun_oggetto;
    if (r < 40) return bicicletta;
    if (r < 65) return bussola;
    if (r < 85) return maglietta_fuocoinferno;
    else return schitarrata_metallica;
}

void stampa_menu(const char* opzioni[], int n_opzioni, int scelta) {
    printf("\n");
    for(int k = 0; k < n_opzioni; k++) {
        if(k == scelta) printf("• %s\n", opzioni[k]);
        else printf("☐ %s\n", opzioni[k]);
    }
}

int gestisci_navigazione(int *scelta, int max_scelta) {
    char k = getch();
    if (k == '\033') {
        getch();
        char f = getch();
        if(f == 'A' && *scelta > 0) (*scelta)--;
        if(f == 'B' && *scelta < max_scelta - 1) (*scelta)++;
        return 0;
    } 
    else if (k == '\n' || k == '\r') return 1;
    return 0;
}


void imposta_gioco() {
    srand(time(NULL));
    undicivirgolacinqueUsato = 0;
    nPlayerMorti = 0;

    for(int i=0; i!=4; i++) {
    statsModificate[i] = 0;
}

    if(gioco_impostato) {
        for(int i=0; i<4; i++) {
            if(player[i]) { free(player[i]); player[i] = NULL; }
        }
        cancella_mappa_esistente();
        gioco_impostato = 0;
    }


    int err = 0;
    do {
        system("clear");
        printf(BLU "-- IMPOSTA GIOCO --\n" STOCK);

        if(err) {
            printf("Il numero di giocatori deve essere compreso tra 1 e 4!\n");
        }

    printf("Per iniziare, inserire il numero di giocatori (Da 1 a 4): ");
    int ess = scanf("%d", &numGiocatori);

    char c;
    while((c = getchar()) != '\n' && c != EOF);

    if (ess != 1 || numGiocatori < 1 || numGiocatori > 4)
            err = 1; 
    else 
            err = 0;

    } while(err);
    


    for(int i=0; i < numGiocatori; i++){
        system("clear");
        printf(BLU "-- IMPOSTA GIOCO --\n" STOCK);
        player[i] = (struct Giocatore*) malloc(sizeof(struct Giocatore));
 
        if (!player[i]) {
            exit(1);
        } 
        
        system("clear");
        printf("%s-- CREAZIONE GIOCATORE %d --%s\n",playerColor[i], i + 1, STOCK);
        player[i]->color = i;
        printf("Nome: ");
        scanf("%49s", player[i]->nome);
        while(getchar() != '\n');
        

        player[i]->hp = 20;
        player[i]->vivo = 1;
        player[i]->mondo = 0;
        player[i]->boost_critico = 0;
        do {
        player[i]->attacco_psichico = (rand() % 16) + 1;
        } while(player[i]->attacco_psichico < 5);
        do {
        player[i]->difesa_psichica = (rand() % 16) + 1;
        } while(player[i]->difesa_psichica < 5);

        do {
        player[i]->fortuna = (rand() % 16) + 1;
         }
        while(player[i]->fortuna < 7);

        for(int k=0; k<3; k++) player[i]->zaino[k] = nessun_oggetto;
        
        
        // Print delle stats
        int scelta_menu = 0;
        static const char* sceltaModifica_opzioni_base[] = {"Modifica Statistiche", "Accetta e Continua"};
        static const char* sceltaModifica_opzioni_ridotte[] = {"Accetta e Continua"};

        // Scelta se modificarle o no


        while(1) {
            system("clear");
            printf("%s-- GIOCATORE %d: %s --\n%s", playerColor[i], i + 1, player[i]->nome, STOCK);
            printf("Attacco: %d | Difesa: %d | Fortuna: %d\n",
                player[i]->attacco_psichico,
                player[i]->difesa_psichica,
                player[i]->fortuna);

                if(statsModificate[i]){
               if (scelta_menu != 0) scelta_menu = 0;
              stampa_menu(sceltaModifica_opzioni_ridotte, 1, scelta_menu);
              if (gestisci_navigazione(&scelta_menu, 1) == 1) {
                break;
              }
            }
              
            else{
            stampa_menu(sceltaModifica_opzioni_base, 2, scelta_menu);
                
                

            if (gestisci_navigazione(&scelta_menu, 2) == 1) {
                // Tasto INVIO premuto
                if (scelta_menu == 0) {
                    modificaStats(i);
                } else {
                    break;
                }
             
            }
        }
        }
        
    } 
    creazioneMappa();
    gioco_impostato = 1;
} 




        void modificaStats(int i) {
        int scelta_mod = 0;
        const char* opzioni_mod[] = {
        "+3 Attacco Psichico | -3 Difesa Psichica",
        "+3 Difesa Psichica | -3 Attacco Psichico",
        "Diventa UndiciVirgolaCinque (+4 Att/Dif | -7 Fortuna)",
        "Annulla"
        };

        while(1) {
        system("clear");
        printf("%s-- MODIFICA STATS: %s --\n%s",playerColor[i], player[i]->nome, STOCK);
       
        printf("Attuali -> Att: %d | Dif: %d | Fort: %d\n", 
               player[i]->attacco_psichico, player[i]->difesa_psichica, player[i]->fortuna);

        
               
        stampa_menu(opzioni_mod, 4, scelta_mod);
        
        if (gestisci_navigazione(&scelta_mod, 4) == 1) {
            switch(scelta_mod) {
                case 0:
                    player[i]->attacco_psichico += 3;
                    player[i]->difesa_psichica -= 3;
                    statsModificate[i] = 1;
                    return; 
                case 1:
                    player[i]->difesa_psichica += 3;
                    player[i]->attacco_psichico -= 3;
                    statsModificate[i] = 1;
                    return;
                case 2:
                    // Logica UndiciVirgolaCinque
                    if(undicivirgolacinqueUsato) {
                        printf(ROSSO"\n\n[!] Esiste gia un UndiciVirgolaCinque! (Max 1 a partita)\n"STOCK);
                        printf("\n\nPremi un tasto per continuare...\n");
                        getch();
                        break;
                    }
                    player[i]->attacco_psichico += 4;
                    player[i]->difesa_psichica += 4;
                    player[i]->fortuna -= 7;
                    strcat(player[i]->nome, "_UndiciVirgolaCinque");
                    statsModificate[i] = 1;
                    undicivirgolacinqueUsato = 1;
                    return;
                case 3:
                    return;
            
        }
    }            
    }
}

static void creazioneMappa() {
    int mappa_chiusa = 0;
    int scelta_mappa = 0;
    const char* opzioni_mappa[] = {
        "Genera Mappa Casuale (15 zone)",
        "Inserisci Zona (Manuale)",
        "Cancella Zona",
        "Stampa Mappa",
        "Stampa Zona Singola",
        "Chiudi Mappa e Gioca"
    };

    while(!mappa_chiusa){
        system("clear");
        printf(BLU "-- CREAZIONE MAPPA --\n" STOCK);
        if (prima_zona_mondoreale == NULL) printf(ROSSO"[!] Mappa non ancora generata.\n"STOCK);
        else printf(VERDE"[✓] Mappa presente in memoria.\n"STOCK);

        stampa_menu(opzioni_mappa, 6, scelta_mappa);

        if (gestisci_navigazione(&scelta_mappa, 6) == 1) {
            switch (scelta_mappa) {
                case 0:

                    if (prima_zona_mondoreale != NULL) {
                        cancella_mappa_esistente();
                    }

                    if(generaMappa()) {
                        printf(VERDE"\n\n[✓] MAPPA GENERATA CON SUCCESSO!\n"STOCK);
                        printf("\n\nPremi un tasto per continuare...\n");
                        getch();
                        }
                    break;
                case 1:
                if (!(prima_zona_mondoreale == NULL)) {
                    inserisciZona(); 
                }
                else {
                    printf(ROSSO"\n\n[!] DEVI PRIMA GENERARE LA MAPPA!\n"STOCK);
                    printf("\n\nPremi un tasto per continuare...\n");
                    getch();
                }
                
                    break;
                case 2:
                    if (!(prima_zona_mondoreale == NULL)) {
                    cancellaZona(); 
                }
                else {
                    printf(ROSSO"\n\n[!] DEVI PRIMA GENERARE LA MAPPA!\n"STOCK);
                    printf("\n\nPremi un tasto per continuare...\n");
                    getch();
                }
                    break;
                case 3:
                    if (!(prima_zona_mondoreale == NULL)) {
                    stampaMappa(); 
                }
                else {
                    printf(ROSSO"\n\n[!] DEVI PRIMA GENERARE LA MAPPA!\n"STOCK);
                    printf("\n\nPremi un tasto per continuare...\n");
                    getch();
                }
                    break;

                case 4:
                    stampaZona();
                    break;
                
                case 5: 
                    if (prima_zona_mondoreale == NULL) {
                        printf(ROSSO"\n\n[!] DEVI PRIMA GENERARE LA MAPPA!\n"STOCK);
                        printf("\n\nPremi un tasto per continuare...\n");
                        getch();
                    } else {
                        int iZone = 0;
                        int iDemo = 0;
                        struct Zona_mondoreale* temp = prima_zona_mondoreale;
                        struct Zona_soprasotto* tempS = prima_zona_soprasotto;

                        while(temp!= NULL) {
                            iZone++;
                            if(tempS->nemico == demotorzone) {
                                iDemo++;
                            }
                            temp = temp->avanti;
                            tempS = tempS->avanti;
                        }

                        if(iZone < 15) {
                            printf(ROSSO"\n\n[!] La mappa deve avere almeno 15 zone\n");
                            printf("\nPremi un tasto per continuare\n");
                            getch();
                        }

                        else if(iDemo != 1) {
                            printf(ROSSO"\n\n[!] La mappa deve contenere ESATTAMENTE un Demotorzone\n");
                            printf("\nPremi un tasto per continuare\n");
                            getch();
                        }
                        else {
                        mappa_chiusa = 1;
                        mappaGenerata = 1;
                        printf(VERDE"\n\n[✓] MAPPA CONFERMATA! Tornando al menu principale...\n"STOCK);
                        printf("\n\nPremi un tasto per continuare...\n");
                        getch();
                        }
                    }
                    break;
            }
    }
}
return;
}

static int generaMappa() {
    int indice_demotorzone = rand() % 15;
    struct Zona_mondoreale* prec_mondoreale = NULL;
    struct Zona_soprasotto* prec_soprasotto = NULL;



    for (int i = 0; i < 15; i++) {
        // Allocazione nodi
        struct Zona_mondoreale* nuovo_mondoreale = (struct Zona_mondoreale*)malloc(sizeof(struct Zona_mondoreale));
        struct Zona_soprasotto* nuovo_soprasotto = (struct Zona_soprasotto*)malloc(sizeof(struct Zona_soprasotto));
        Tipo_zona tipo = (Tipo_zona)(rand() % 10);
        nuovo_mondoreale->tipo = tipo;
        nuovo_soprasotto->tipo = tipo;

        nuovo_mondoreale->nemico = random_nemico_mondoreale();
        nuovo_mondoreale->oggetto = random_oggetto();

        if (i == indice_demotorzone) {
            nuovo_soprasotto->nemico = demotorzone;
        } else {
            nuovo_soprasotto->nemico = random_nemico_soprasotto();
        }
        nuovo_mondoreale->link_soprasotto = nuovo_soprasotto;
        nuovo_soprasotto->link_mondoreale = nuovo_mondoreale;

        nuovo_mondoreale->avanti = NULL;
        nuovo_soprasotto->avanti = NULL;

        if (i == 0) {
            prima_zona_mondoreale = nuovo_mondoreale;
            prima_zona_soprasotto = nuovo_soprasotto;
            nuovo_mondoreale->indietro = NULL;
            nuovo_soprasotto->indietro = NULL;
        } else {
            prec_mondoreale->avanti = nuovo_mondoreale;
            nuovo_mondoreale->indietro = prec_mondoreale;

            prec_soprasotto->avanti = nuovo_soprasotto;
            nuovo_soprasotto->indietro = prec_soprasotto;
        }

        prec_mondoreale = nuovo_mondoreale;
        prec_soprasotto = nuovo_soprasotto;
    }
        return 1;
}

static int inserisciZona() {
    int posNuovaZona = -1;
    
    // 1. Input Posizione (Questo lo lasciamo numerico perché scorrere 15 numeri con le frecce è noioso)
    do {
        system("clear");
        printf("-- INSERISCI ZONA --\n");
        printf("\n Inserisci una posizione in cui inserire la nuova zona (Min. 1): ");
        if (scanf("%d", &posNuovaZona) != 1) {
            while(getchar() != '\n'); 
            posNuovaZona = -1;      
        }
    } while(posNuovaZona < 1);

    int index = posNuovaZona - 1;

    // 2. Allocazione memoria
    struct Zona_mondoreale* nuovo_mondoreale = (struct Zona_mondoreale*)malloc(sizeof(struct Zona_mondoreale));
    struct Zona_soprasotto* nuovo_soprasotto = (struct Zona_soprasotto*)malloc(sizeof(struct Zona_soprasotto));

    if (!nuovo_mondoreale || !nuovo_soprasotto) {
        printf("Errore memoria.\n");
        return 0;
    }

    // 3. Generazione Tipo Casuale (Rimane random come da specifiche)
    Tipo_zona tipo = (Tipo_zona)(rand() % 10);
    nuovo_mondoreale->tipo = tipo;
    nuovo_soprasotto->tipo = tipo;

    // --- NUOVA GESTIONE INPUT CON FRECCETTE ---
    
    int scelta_temp = 0;

    // A. Scelta Nemico Mondo Reale
    // Opzioni corrispondenti all'enum: 0=Nessuno, 1=Billi, 2=Democane
    const char* menu_nemici_mr[] = {"Nessuno", "Billi", "Democane"};
    while(1) {
        system("clear");
        printf("-- ZONA %d (Mondo Reale) --\n", posNuovaZona);
        printf("Scegli il Nemico:\n");
        stampa_menu(menu_nemici_mr, 3, scelta_temp);
        if (gestisci_navigazione(&scelta_temp, 3) == 1) break;
    }
    nuovo_mondoreale->nemico = (Tipo_nemico)scelta_temp; // Mappatura diretta (0,1,2)

    // B. Scelta Oggetto Mondo Reale
    scelta_temp = 0; // Reset scelta
    const char* menu_oggetti[] = {"Nessuno", "Bicicletta", "Maglietta Fuocoinferno", "Bussola", "Schitarrata Metallica"};
    while(1) {
        system("clear");
        printf("-- ZONA %d (Mondo Reale) --\n", posNuovaZona);
        printf("Scegli l'Oggetto:\n");
        stampa_menu(menu_oggetti, 5, scelta_temp);
        if (gestisci_navigazione(&scelta_temp, 5) == 1) break;
    }
    nuovo_mondoreale->oggetto = (Tipo_oggetto)scelta_temp; // Mappatura diretta

    // C. Scelta Nemico Soprasotto
    // Opzioni visibili: Nessuno, Democane, Demotorzone (Billi non c'è)
    scelta_temp = 0; // Reset scelta
    const char* menu_nemici_ss[] = {"Nessuno", "Democane", "Demotorzone"};
    while(1) {
        system("clear");
        printf("-- ZONA %d (Soprasotto) --\n", posNuovaZona);
        printf("Scegli il Nemico:\n");
        stampa_menu(menu_nemici_ss, 3, scelta_temp);
        if (gestisci_navigazione(&scelta_temp, 3) == 1) break;
    }
    
    // Mappatura manuale per il Soprasotto perché saltiamo "Billi"
    if (scelta_temp == 0) nuovo_soprasotto->nemico = nessun_nemico;
    else if (scelta_temp == 1) nuovo_soprasotto->nemico = democane;
    else if (scelta_temp == 2) nuovo_soprasotto->nemico = demotorzone;

    // ------------------------------------------

    // 5. Collegamento Verticale
    nuovo_mondoreale->link_soprasotto = nuovo_soprasotto;
    nuovo_soprasotto->link_mondoreale = nuovo_mondoreale;

    // 6. Inserimento nella Lista (Linking Orizzontale)
    if (index == 0) {
        nuovo_mondoreale->avanti = prima_zona_mondoreale;
        nuovo_soprasotto->avanti = prima_zona_soprasotto;
        nuovo_mondoreale->indietro = NULL;
        nuovo_soprasotto->indietro = NULL;

        if (prima_zona_mondoreale != NULL) prima_zona_mondoreale->indietro = nuovo_mondoreale;
        if (prima_zona_soprasotto != NULL) prima_zona_soprasotto->indietro = nuovo_soprasotto;

        prima_zona_mondoreale = nuovo_mondoreale;
        prima_zona_soprasotto = nuovo_soprasotto;
    }
    else {
        struct Zona_mondoreale* corr_mr = prima_zona_mondoreale;
        struct Zona_soprasotto* corr_ss = prima_zona_soprasotto;
        
        int i = 0;
        while (i < index - 1 && corr_mr != NULL && corr_mr->avanti != NULL) {
            corr_mr = corr_mr->avanti;
            corr_ss = corr_ss->avanti;
            i++;
        }
        
        nuovo_mondoreale->avanti = corr_mr->avanti;
        nuovo_soprasotto->avanti = corr_ss->avanti;

        if (corr_mr->avanti != NULL) corr_mr->avanti->indietro = nuovo_mondoreale;
        if (corr_ss->avanti != NULL) corr_ss->avanti->indietro = nuovo_soprasotto;

        corr_mr->avanti = nuovo_mondoreale;
        corr_ss->avanti = nuovo_soprasotto;

        nuovo_mondoreale->indietro = corr_mr;
        nuovo_soprasotto->indietro = corr_ss;
    }

    printf(VERDE"\n[✓] Zona inserita con successo in posizione %d!\n", posNuovaZona);
    fflush(stdout);
    printf("\n\nPremi un tasto per continuare...\n");
    getch();
    return 1;
}

static void cancellaZona() {
    int posDaCancellare = -1;
    
    // Controlliamo se la mappa è vuota
    if (prima_zona_mondoreale == NULL) {
        printf(ROSSO"\n[!] Errore: Nessuna mappa da cancellare.\n"STOCK);
        printf("\n\nPremi un tasto per continuare...\n");
        getch();
        
        return;
    }

    do {
        system("clear");
        printf("-- CANCELLA ZONA --\n");
        printf("\nInserisci la posizione della zona da cancellare (Min. 1): ");
        if (scanf("%d", &posDaCancellare) != 1) {
            while(getchar() != '\n'); 
            posDaCancellare = -1;      
        }
    } while(posDaCancellare < 1);

    int index = posDaCancellare - 1;
    struct Zona_mondoreale* target_mr = prima_zona_mondoreale;
    struct Zona_soprasotto* target_ss = prima_zona_soprasotto;

    // 1. Trova la zona target
    int i = 0;
    while (i < index && target_mr != NULL) {
        target_mr = target_mr->avanti;
        target_ss = target_ss->avanti;
        i++;
    }

    if (target_mr == NULL) {
        printf(ROSSO"\n[!] Errore: La zona %d non esiste!\n"STOCK, posDaCancellare);
        printf("\n\nPremi un tasto per continuare...\n");
        getch();
        return;
    }

    
    // Cancellazione della Testa
    if (target_mr == prima_zona_mondoreale) {
        prima_zona_mondoreale = target_mr->avanti;
        prima_zona_soprasotto = target_ss->avanti;
        
        if (prima_zona_mondoreale != NULL) prima_zona_mondoreale->indietro = NULL;
        if (prima_zona_soprasotto != NULL) prima_zona_soprasotto->indietro = NULL;
    }
    // Cancellazione nel mezzo o coda
    else {
        target_mr->indietro->avanti = target_mr->avanti;
        target_ss->indietro->avanti = target_ss->avanti;

        if (target_mr->avanti != NULL) {
            target_mr->avanti->indietro = target_mr->indietro;
            target_ss->avanti->indietro = target_ss->indietro;
        }
    }

    // 3. Deallocazione memoria
    free(target_mr);
    free(target_ss);

    printf(VERDE"\n[✓] Zona %d cancellata con successo.\n"STOCK, posDaCancellare);
    fflush(stdout);
    printf("\n\nPremi un tasto per continuare...\n");
    getch();
}


static const char* nome_zona(Tipo_zona z) {
    switch(z) {
        case bosco: return "Bosco";
        case scuola: return "Scuola";
        case laboratorio: return "Laboratorio";
        case caverna: return "Caverna";
        case strada: return "Strada";
        case giardino: return "Giardino";
        case supermercato: return "Supermercato";
        case centrale_elettrica: return "Centrale Elettrica";
        case deposito_abbandonato: return "Deposito";
        case stazione_polizia: return "Polizia";
        default: return "???";
    }
}

static const char* nome_nemico(Tipo_nemico n) {
    switch(n) {
        case nessun_nemico: return "Nessuno";
        case billi: return "Billi";
        case democane: return "Democane";
        case demotorzone: return "DEMOTORZONE";
        default: return "???";
    }
}

static const char* nome_oggetto(Tipo_oggetto o) {
    switch(o) {
        case nessun_oggetto: return "Nessuno";
        case bicicletta: return "Bicicletta";
        case maglietta_fuocoinferno: return "Maglietta Fuoco";
        case bussola: return "Bussola";
        case schitarrata_metallica: return "Schitarrata Metallica";
        default: return "???";
    }
}

static void stampaZona() {
    if (prima_zona_mondoreale == NULL) {
        printf(ROSSO"\n[!] Nessuna mappa presente.\n"STOCK);
        printf("\n\nPremi un tasto per continuare...\n");
        getch();
        return;
    }

    int pos = -1;
    do {
        system("clear");
        printf(GIALLO"-- STAMPA ZONA SINGOLA --\n"STOCK);
        printf("\nInserisci il numero della zona da visualizzare (Min. 1): ");
        if (scanf("%d", &pos) != 1) {
            while(getchar() != '\n'); 
            pos = -1;      
        }
    } while(pos < 1);

    struct Zona_mondoreale* temp_mr = prima_zona_mondoreale;
    struct Zona_soprasotto* temp_ss = prima_zona_soprasotto;
    int i = 1;

    // Scorriamo la lista fino alla posizione richiesta
    while (temp_mr != NULL && i < pos) {
        temp_mr = temp_mr->avanti;
        temp_ss = temp_ss->avanti;
        i++;
    }

    if (temp_mr == NULL) {
        printf(ROSSO"\n\n[!] La zona %d non esiste!\n"STOCK, pos);
    } else {
        printf(CIANO"\n\n--- DETTAGLI ZONA %d ---\n"STOCK, pos);
        printf(VERDE"Mondo Reale:"STOCK" [%s]\n", nome_zona(temp_mr->tipo));
        printf("   - Nemico:  %s\n", nome_nemico(temp_mr->nemico));
        printf("   - Oggetto: %s\n", nome_oggetto(temp_mr->oggetto));
        
        printf(ROSSO"Soprasotto:"STOCK"  [%s]\n", nome_zona(temp_ss->tipo));
        printf("   - Nemico:  %s\n", nome_nemico(temp_ss->nemico));
    }

    printf("\n\nPremi un tasto per continuare...\n");
    getch();
}

static void stampaMappa() {
    if (prima_zona_mondoreale == NULL) {
        printf(ROSSO"\n[!] Nessuna mappa presente.\n"STOCK);
        printf("\n\nPremi un tasto per continuare...\n");
        getch();
        return;
    }

    // Chiediamo quale mondo stampare usando le freccette
    int scelta_mondo = 0;
    const char* menu_mondi[] = {"Mondo Reale", "Soprasotto", "Entrambi (Vista Completa)"};
    
    while(1) {
        system("clear");
        printf("-- STAMPA MAPPA --\n");
        printf("Scegli quale dimensione visualizzare:\n");
        stampa_menu(menu_mondi, 3, scelta_mondo);
        if (gestisci_navigazione(&scelta_mondo, 3) == 1) break;
    }

    
    

    struct Zona_mondoreale* temp_mr = prima_zona_mondoreale;
    struct Zona_soprasotto* temp_ss = prima_zona_soprasotto;
    int contatore = 1;
    system("clear");
    printf("\n\n--- MAPPA DI OCCHINZ ---\n\n");
    while (temp_mr != NULL) {
        printf("ZONA %d: [%s]\n", contatore, nome_zona(temp_mr->tipo));
        
        if (scelta_mondo == 0 || scelta_mondo == 2) {
            printf("   Reale:      Nemico: %-10s | Oggetto: %s\n", 
                   nome_nemico(temp_mr->nemico), nome_oggetto(temp_mr->oggetto));
        }
        
        if (scelta_mondo == 1 || scelta_mondo == 2) {
            printf("   Soprasotto: Nemico: %-10s\n", nome_nemico(temp_ss->nemico));
        }
        
        printf("   --------------------------------------------------\n");

        temp_mr = temp_mr->avanti;
        temp_ss = temp_ss->avanti;
        contatore++;
    }

    printf("\nPremi un tasto per tornare al menu...");
    getch();
}


        

void gioca() {
    system("clear");
    if(!gioco_impostato) {
        printf(ROSSO"[!] Devi prima impostare il gioco!"STOCK);
        printf("\n\nPremi un tasto per continuare...\n");
        getch();
        return;
    }

    for(int i = 0; i< numGiocatori; i++) {
        player[i]->mondo = 0; 
        player[i]->pos_mondoreale = prima_zona_mondoreale;
        player[i]->pos_soprasotto = prima_zona_soprasotto;
    }

    int giocoInCorso = 1;
    int numeroRound = 1; 
    demotorzoneSconfitto = 0;

    while(giocoInCorso) {
        system("clear");
        if(nPlayerMorti == numGiocatori) {
            printf(ROSSO"[!] Tutti i giocatori sono morti, l'oscurità ha vinto... Sta volta."STOCK);
            printf("\n\nPremi un tasto per continuare...\n");
            getch();
            return;
        }

        printf("-- INIZIO ROUND N.%d --\n\n", numeroRound);

        int ordineTurno[4]; 
        for (int k = 0; k < numGiocatori; k++) {
            ordineTurno[k] = k;
        }

        for (int k = numGiocatori - 1; k > 0; k--) {
            int j = rand() % (k + 1); 
            int temp = ordineTurno[k];
            ordineTurno[k] = ordineTurno[j];
            ordineTurno[j] = temp;
        }

        for(int k = 0; k < numGiocatori; k++){
            int idCorrente = ordineTurno[k]; 
            
            if(player[idCorrente]->vivo == 1) {
            
                printf("- GIOCATORE %d -\n", idCorrente + 1); 
                giocaTurno(idCorrente); 

                if(demotorzoneSconfitto) {
                    int kk=0;
                    for(int frame = 0; frame < 40; frame++) {
                    system("clear");
                    printf(GIALLO"\n\n*****************************************\n"STOCK);
                    printf(VERDE"   IL DEMOTORZONE È STATO SCONFITTO!\n"STOCK);
                    printf(GIALLO"*****************************************\n"STOCK);
                    printf("\nGloria eterna a %s%s%s, l'eroe di Occhinz!\n", allColors[kk], player[idCorrente]->nome, STOCK);

                    usleep(75000); 
                    kk++;
                    if(kk >= 20) kk = 0;
                    }

                    strcpy(storicoVincitori[2],storicoVincitori[1]);
                    strcpy(storicoVincitori[1],storicoVincitori[0]);
                    strcpy(storicoVincitori[0],player[idCorrente]->nome);
                    

                    giocoInCorso = 0; 
                    gioco_impostato = 0;
                    printf(GIALLO"\nHai salvato la città dall'oscurità.\n"STOCK);
                    printf("\nPremi un tasto per terminare la partita...\n");
                    getch();
                    break;
                    
            }
                
            }
        }

        if(!giocoInCorso) 
            break;

        system("clear");
        printf(GIALLO"-- RIEPILOGO TURNO --\n\n");
        printf(VERDE"[✓] Turno %d completato!\n"STOCK, numeroRound);
        
        
        nessunMorto = 1; 
        for(int i=0; i<numGiocatori; i++) {
            if(player[i]->vivo == 0) {
                nessunMorto = 0;
            }
        }

        if(nessunMorto)
            printf(VERDE"\n[✓] Tutti i giocatori sono ancora in gioco!\n"STOCK);
        
        else{
            for(int i=0; i<numGiocatori; i++) {
            if(player[i]->vivo == 0)
            printf(ROSSO"\n[!] Il Giocatore %s è morto!\n"STOCK, player[i]->nome);
            }
        }

        printf("\nPremi un tasto per passare al prossimo round...\n");
        getch();
        numeroRound++;

        
    }

    termina_gioco();
} 


static void giocaTurno(int indGiocatore) {
    struct Giocatore* p = player[indGiocatore];
    int turnoFinito = 0;
    int sceltaAzione = 0;
    int nemicoStordito = 0;
    int biciAttiva = 0;

    

    const char* azioniSenzaNemico[] = {
        "Avanza",
        "Indietreggia",
        "Cambia Mondo",
        "Raccogli Oggetto",
        "Apri Zaino",
        "Info Giocatore",
        "Passa turno"
    };





    while(!turnoFinito) {
        system("clear");
        
        int numZona = 1;
        struct Zona_mondoreale* temp = prima_zona_mondoreale;
        while(temp != NULL && temp != p->pos_mondoreale) {
            temp = temp->avanti;
            numZona++;
        }

        const char* nome_zona_attuale = "???";
        Tipo_nemico nemico_qui = nessun_nemico;


            if(p->mondo == 0) {
                if (p->pos_mondoreale != NULL) {
                nome_zona_attuale = nome_zona(p->pos_mondoreale->tipo);
                nemico_qui = p->pos_mondoreale->nemico;
            }
            }
            else if(p->pos_soprasotto != NULL) {
                nome_zona_attuale = nome_zona(p->pos_soprasotto->tipo);
                nemico_qui = p->pos_soprasotto->nemico;
            }

            if(nemicoStordito || biciAttiva) {
            nemico_qui = nessun_nemico; 
        }


        printf("%s--- TURNO DI: %s ---\n" STOCK, playerColor[p->color], p->nome);
        printf("Posizione: %s, %s - (Zona %d)\n", 
               ((p->mondo == 0) ? VERDE"Mondo Reale" : ROSSO"Soprasotto"), nome_zona_attuale, numZona);

        
        if(nemico_qui != nessun_nemico) {
            printf(ROSSO"[!] Nella stanza c'è un %s!\n"STOCK, nome_nemico(nemico_qui));
            if(p->mondo == 0)
            printf("\nOggetto misterioso! Sconfiggi il nemico per rivelarlo\n");
        }                           
        else {
            if(nemicoStordito) 
                printf(GIALLO"\n\n[!] Il nemico è a terra, stordito! Hai via libera... per ora.\n"STOCK);
                else if(biciAttiva)     
                    printf(MAGENTA"\n\n[!] Sei in sella alla tua bici, sei troppo veloce per il nemico!\n"STOCK);
                
            else 
                printf(VERDE"\n\n[✓] E' tutto così calmo qui... Nessuna minaccia in vista\n"STOCK);
            
            if(p->mondo == 0 && !biciAttiva)
            printf("Oggetto presente: %s%s%s\n",GIALLO, nome_oggetto(p->pos_mondoreale->oggetto), STOCK);
            else if(biciAttiva) 
            printf("Scappando dal nemico non riesci a capire che oggetto ci sia...\n");
        }
    
        const char* azioniConNemico[] = {
        "Combatti",
        "Scappa",
        "Info Giocatore",
        "Cambia Mondo (Solo Soprasotto)"
    };


        if(nemico_qui != nessun_nemico) 
        stampa_menu(azioniConNemico, 4, sceltaAzione);

        else
        stampa_menu(azioniSenzaNemico, 7, sceltaAzione);


        // GESTIONE SCELTE SENZA NEMICO 
        if(nemico_qui == nessun_nemico) {
        if (gestisci_navigazione(&sceltaAzione, 7) == 1) {
            switch(sceltaAzione) {
                case 0: // AVANZA
                    if (p->mondo == 0) { 
                        if (p->pos_mondoreale->avanti != NULL) {
                            p->pos_mondoreale = p->pos_mondoreale->avanti;
                            p->pos_soprasotto = p->pos_mondoreale->link_soprasotto;
                            printf(VERDE"\n[✓] Ti sei spostato in avanti!\n"STOCK);
                            turnoFinito = 1;
                        } else {
                            printf(ROSSO"\n[!] Sei all'ultima zona! Non puoi avanzare oltre.\n");
                        }
                    } 
                    else {
                        if (p->pos_soprasotto->avanti != NULL) {
                            p->pos_soprasotto = p->pos_soprasotto->avanti;
                            p->pos_mondoreale = p->pos_soprasotto->link_mondoreale;
                            printf(ROSSO"\n[✓] Ti sei spostato in avanti (nel buio...)\n"STOCK);
                            turnoFinito = 1;
                        } else {
                            printf("\n[!] Sei alla fine del percorso.\n");
                        }
                    }
                    printf("\n\nPremi un tasto per continuare...\n");
                    getch();
                    break;

                case 1: // INDIETREGGIA
                if(p->mondo == 0) {
                    if(p->pos_mondoreale->indietro == NULL) {
                        printf(ROSSO"\n\n[!] Sei alla prima zona, non puoi indietreggiare!"STOCK);
                        printf("\n\nPremi un tasto per continuare...\n");
                        getch();
                        break;
                    }
                    else{
                        p->pos_mondoreale = p->pos_mondoreale->indietro;
                        p->pos_soprasotto = p->pos_mondoreale->link_soprasotto;
                        printf(VERDE"\n\n[✓] Sei tornato indietro alla zona precedente!\n" STOCK);
                        turnoFinito = 1;
                        printf("\n\nPremi un tasto per continuare...\n");
                        getch();
                        break;

                    }

                }

                if(p->mondo == 1) {
                    if(p->pos_soprasotto->indietro == NULL) {
                        printf(ROSSO"\n\n[!] Sei alla prima zona, non puoi indietreggiare!"STOCK);
                        printf("\n\nPremi un tasto per continuare...\n");
                        getch();
                        break;
                    }
                    else {
                        p->pos_soprasotto = p->pos_soprasotto->indietro;
                        p->pos_mondoreale = p->pos_soprasotto->link_mondoreale;
                        printf(VERDE"\n\n[✓] Sei tornato indietro alla zona precedente!" STOCK);
                        turnoFinito = 1;
                        printf("\n\nPremi un tasto per continuare...\n");
                        getch();
                        break;
                    }
                }
                
                
                
                break;

                case 2: 
                    if(p->mondo == 0) {
                        p->mondo = 1;
                        printf(ROSSO"\n\n[!] Hai preso il portale per il temibile SopraSotto!" STOCK);
                        turnoFinito = 1;
                        printf("\n\nPremi un tasto per continuare...\n");
                        getch();
                        break;
                    }

                    if(p->mondo == 1) {
                        int dadoFuga = (rand()%20)+1;
                        printf(GIALLO"\n\n[!] Tenti di scappare dal soprasotto...\n" STOCK);
                        if(dadoFuga < p->fortuna) {
                        p->mondo = 0;
                        turnoFinito = 1;
                        printf(VERDE"\n[✓] Ce l'hai fatta! Prendi il portale e torni al mondo reale!\n"STOCK);
                        printf("\n\nPremi un tasto per continuare...\n");
                        getch();
                        break;
                        }
                        else {
                            turnoFinito = 1;
                            printf(ROSSO"\n[!] Molto male... Il portale si chiude lasciandoti nel soprasotto...\n"STOCK);
                            printf("\n\nPremi un tasto per continuare...\n");
                            getch();
                            break;

                        }
                    }
            
            
            case 3: // RACCOGLI OGGETTO
                    if(biciAttiva) {
                        printf(ROSSO"\n\n[!] Non puoi raccogliere oggetti mentre sei in bicicletta!\n"STOCK);
                            printf(GIALLO"    Il nemico ti sbarra la strada se ti fermi.\n"STOCK);
                            printf("\nPremi un tasto per continuare...\n");
                            getch();
                            break;
                    }
                    // 1. Controllo se c'è un oggetto nella stanza
                    if(p->pos_mondoreale->oggetto == nessun_oggetto || p->mondo == 1) {
                        printf(ROSSO"\n\n[!] Non c'è nessun oggetto qui!\n"STOCK);
                        printf("\n\nPremi un tasto per continuare...\n");
                        getch();
                        break; 
                    }

                    // 2. Controllo se lo zaino è pieno
                    int zainoPieno = 1;
                    int slotLibero = -1;
                    for(int k=0; k<3; k++) {
                        if(p->zaino[k] == nessun_oggetto) {
                            zainoPieno = 0;
                            slotLibero = k;
                            break;
                        }
                    }

                    // CASO A: ZAINO PIENO - DEVI SCAMBIARE
                    if(zainoPieno) {
                        int sceltaLascia = 0;
                        int operazioneConclusa = 0;

                        while(!operazioneConclusa) {
                            system("clear");
                            printf(ROSSO"-- ZAINO PIENO --\n"STOCK);
                            printf("Hai trovato: %s\n", nome_oggetto(p->pos_mondoreale->oggetto));
                            printf("Scegli quale oggetto buttare per fare spazio:\n");

                            // Creiamo il menu dinamico
                            const char* menuZainoPieno[] = {
                                nome_oggetto(p->zaino[0]),
                                nome_oggetto(p->zaino[1]),
                                nome_oggetto(p->zaino[2]),
                                "Annulla (Lascia l'oggetto a terra)"
                            };

                            stampa_menu(menuZainoPieno, 4, sceltaLascia);

                            if (gestisci_navigazione(&sceltaLascia, 4) == 1) {
                                // Se preme INVIO
                                if(sceltaLascia == 3) {
                                    // Ha scelto Annulla
                                    printf(GIALLO"\n[!] Hai deciso di lasciare l'oggetto dov'era.\n"STOCK);
                                    operazioneConclusa = 1;
                                } 
                                else {
                                    // Ha scelto uno slot (0, 1 o 2)
                                    printf(VERDE"\n\n[✓] Hai buttato %s e raccolto %s!\n"STOCK, 
                                           nome_oggetto(p->zaino[sceltaLascia]), 
                                           nome_oggetto(p->pos_mondoreale->oggetto));
                                    
                                    // Scambio effettivo
                                    p->zaino[sceltaLascia] = p->pos_mondoreale->oggetto;
                                    p->pos_mondoreale->oggetto = nessun_oggetto; // Rimuove da terra
                                    
                                    operazioneConclusa = 1;
                                }
                                printf("\n\nPremi un tasto per continuare...\n");
                                getch();
                            }
                        }
                    } 
                    // CASO B: C'È SPAZIO NELLO ZAINO
                    else {
                        p->zaino[slotLibero] = p->pos_mondoreale->oggetto;
                        printf(VERDE"\n\n[✓] Hai raccolto: %s\n"STOCK, nome_oggetto(p->pos_mondoreale->oggetto));
                        p->pos_mondoreale->oggetto = nessun_oggetto; // Rimuove da terra
                        
                        printf("\n\nPremi un tasto per continuare...\n");
                        getch();
                    }
                    break;

                case 4: 
                apriZaino(p);
                break;

                case 5: // INFO GIOCATORE
                    printf(GIALLO"\n--- STATISTICHE ---\n"STOCK);
                    printf("Salute: %d\n", p->hp);
                    printf("Attacco: %d\n", p->attacco_psichico);
                    printf("Difesa:  %d\n", p->difesa_psichica);
                    printf("Fortuna: %d\n", p->fortuna);
                    printf(GIALLO"\n\n--- ZAINO ---\n"STOCK);
                    for(int i=0; i<3; i++){
                        if(p->zaino[i] == nessun_oggetto)
                            printf("SLOT VUOTO\n");
                        else
                            printf("%s\n", nome_oggetto(p->zaino[i]));
                    }
                    printf("\nPremi un tasto per tornare al menu...\n");
                    getch();
                    break;

                case 6: // PASSA TURNO
                if(nemico_qui == nessun_nemico) {
                    printf(VERDE"\n[✓] Decidi di riposare e passi il turno.\n"STOCK);
                    printf("\n\nPremi un tasto per continuare...\n");
                    getch();
                    turnoFinito = 1;
                    break;
                }
                else {
                    printf(ROSSO"[!] Non puoi passare il turno, prima devi vedertela con %s!"STOCK, nome_nemico(nemico_qui));
                }
            }
        }
    }

    else if(nemico_qui != nessun_nemico){
         if (gestisci_navigazione(&sceltaAzione, 4) == 1) {
            switch(sceltaAzione) {
                case 0: { // COMBATTI 
                int esitoFight = combatti(p, nemico_qui);
                    if(esitoFight == 1) {
                        if(demotorzoneSconfitto == 1) {
                        if(p->mondo == 0)   
                            p->pos_mondoreale->nemico = nessun_nemico;
                        else
                             p->pos_soprasotto->nemico = nessun_nemico;
                        turnoFinito = 1;
                        return;
                        }
                        int nemicoScompare = (rand() % 2);
                        if(nemicoScompare == 1){
                        if(p->mondo == 0)
                        p->pos_mondoreale->nemico = nessun_nemico;
                        else 
                        p->pos_soprasotto->nemico = nessun_nemico;
                        nemico_qui = nessun_nemico;
                        printf(VERDE"\n[✓] Il nemico non si rialza, la zona è stata ripulita!\n"STOCK);
                        printf("\n\nPremi un tasto per continuare...\n");
                        getch();       
                        }
                    else {
                        printf(GIALLO"\n[!] Il nemico è a terra ma respira ancora, approfittane per agire!\n"STOCK);
                        nemicoStordito = 1;
                        printf("\n\nPremi un tasto per continuare...\n");
                        getch();  
                    }
                    }
                    if(esitoFight == 0) {
                        p->vivo = 0;
                        turnoFinito = 1;
                        nPlayerMorti++;
                    }

                    if(esitoFight == 2) {
                        biciAttiva = 1;
                        break;
                    }
                    getch();
                    break;
                }
                case 1:
                {
                int biciTrovata = 0;
                for(int i = 0; i<3; i++) {
                if(p->zaino[i] == bicicletta) {
                   printf(GIALLO"\n\n[!] Per scappare, apri lo zaino in combattimento e seleziona la Bicicletta!\n"STOCK);
                   biciTrovata = 1;
                   printf("\n\nPremi un tasto per continuare...\n");
                   getch();
                   break;
                }
            }  

            if(!biciTrovata) {
                printf(ROSSO"\n\n[!] Non puoi scappare, non hai la bicicletta!\n"STOCK);
                printf("\n\nPremi un tasto per continuare...\n");
                getch();
            }
            break;
        }

                case 2: // INFO GIOCATORE
                    printf(GIALLO"\n--- STATISTICHE ---\n"STOCK);
                    printf("Salute: %d\n", p->hp);
                    printf("Attacco: %d\n", p->attacco_psichico);
                    printf("Difesa:  %d\n", p->difesa_psichica);
                    printf("Fortuna: %d\n", p->fortuna);
                    printf("\nPremi un tasto per tornare al menu...");
                    getch();
                    break;

                
                case 3:
                if(p->mondo == 0) {
                    printf(ROSSO"\n\n[!] Non puoi cambiare mondo! Devi prima sconfiggere il nemico che ti blocca la strada!\n"STOCK);
                        printf("\nPremi un tasto per continuare...\n");
                        getch();
                    }

                else {
                    int dadoFuga = (rand()%20)+1;
                    printf(GIALLO"\n\n[!] Sei nel Soprasotto! Tenti di aprire il portale per fuggire dal nemico...\n" STOCK);
                    if(dadoFuga < p->fortuna) {
                            p->mondo = 0;
                            turnoFinito = 1;
                            printf(VERDE"\n[✓] Incredibile! Il portale si apre e fuggi nel Mondo Reale!\n"STOCK);
                            printf("\n\nPremi un tasto per continuare...\n");
                            getch();
                        }
                        else {
                            turnoFinito = 1; // Fallimento, perdi il turno
                            printf(ROSSO"\n[!] Il portale resta chiuso! Sei intrappolato col nemico...\n"STOCK);
                            printf("\n\nPremi un tasto per continuare...\n");
                            getch();
                        }
                    }
                    break;

                }
            }
        }
    }  
}

 static int combatti(struct Giocatore* p, Tipo_nemico nemicoQui) {
    int hpRefill = 0;
    int atkPlayer;
    int atkRoll;
    int combattimentoFinito = 0;
    int hpNemico = 0;
    if (nemicoQui == democane)
        hpNemico = 20;
    if (nemicoQui == billi) 
        hpNemico = 30;
    if (nemicoQui == demotorzone)
        hpNemico = 50; 

        system("clear");
        printf(ROSSO"-- COMBATTIMENTO --\n\n"STOCK);

        

    const char* azioniCombattimento[] = {
        "Attacca",
        "Apri Zaino",
        "Cambia Mondo (Solo Soprasotto)"
    };
    int sceltaCombattimento = 0;
    

    //ATTACCO NEMICO
    while(!combattimentoFinito) {
    system("clear");
    printf(GIALLO"-- COMBATTIMENTO --\n"STOCK);
    printf("\nNemico: %s%s%s\n", ROSSO, nome_nemico(nemicoQui), STOCK);
    printf("\nHP Giocatore: %s%d%s\n",MAGENTA,p->hp,STOCK);
    printf("HP Nemico: %s%d%s\n",ROSSO, hpNemico, STOCK);

    stampa_menu(azioniCombattimento, 2, sceltaCombattimento);
    if (gestisci_navigazione(&sceltaCombattimento, 2) == 1) {
            switch(sceltaCombattimento) {
                case 0:
                    atkRoll = (rand() % 20) + 1;


                    //printf("\n\n[!] DEBUG -- PRE FORTUNE ROLL: %d\n\n", atkRoll);
                    atkRoll += (p->fortuna/4);

                    // printf("\n\n[!] DEBUG -- ROLL: %d\n\n", atkRoll);
                    if(p->boost_critico == 1) {
                        printf(MAGENTA"\n[!] La Schitarrata Metallica guida il tuo colpo!\n"STOCK);
                        atkPlayer = p->attacco_psichico*2; 
                        printf(GIALLO"\n\n[CRITICO!] L'attacco è Superefficace e inflige %d HP al nemico!", atkPlayer);
                        hpNemico -= atkPlayer;
                        p->boost_critico = 0;
                        break;
                    }

                    if(atkRoll < 8) {
                        printf(ROSSO"\n\n[!] Attacco Mancato! Hai perso la concentrazione nel momento critico!"STOCK);
                        break;
                    }

                    if(atkRoll < 17) {
                        atkPlayer = (p->attacco_psichico + (rand() % 3))/2;
                        printf(VERDE"\n\n[✓] L'attacco va a segno e inflige %d HP al nemico!"STOCK, atkPlayer);
                        //printf("\n\n[!] DEBUG -- ATK: %d", atkPlayer);
                        hpNemico -= atkPlayer;
                        break;
                    } 

                    if(atkRoll >= 17) {
                        atkPlayer = p->attacco_psichico + (rand() % 3);
                        printf(GIALLO"\n\n[CRITICO!] L'attacco è Superefficace e inflige %d HP al nemico!", atkPlayer);
                        hpNemico -= atkPlayer;
                        break;
                    }
                
                case 1: {
                    int oggettoZaino = apriZaino(p);
                    if(!oggettoZaino)
                        continue;
                    if(oggettoZaino == 1) 
                        break;
                    if(oggettoZaino == 2) { 
                        return 2;
                        }
                    }
                    break;
                }
        if(hpNemico > 0) {
    int atkNemico = (rand() % 20);
    if(atkNemico <= p->difesa_psichica) {
        printf(VERDE"\n\n[✓] L'attacco del nemico non ha avuto effetto!\n"STOCK);
        }
    else {
        p->hp = p->hp-(atkNemico - p->difesa_psichica);
        printf(ROSSO"\n\n[!] L'attacco del nemico ti inflige %d HP!\n"STOCK, atkNemico - p->difesa_psichica);
        if(p->hp < 0) {
            p->hp = 0;
    }   
}

        printf("\nHP Giocatore: %s%d%s\n",MAGENTA,p->hp,STOCK);
    printf("HP Nemico: %s%d%s\n",ROSSO, hpNemico, STOCK);
        printf("\nPremi un tasto per continuare\n");
        getch();
    
}
    if(p->hp <= 0) {
        printf(ROSSO"\n\n[!] Il Giocatore %s è morto!"STOCK, p->nome);
        combattimentoFinito = 1;
        printf("\n\nPremi un tasto per continuare");
        getch();
        return 0;
        }
    if(hpNemico <= 0) {
        if(nemicoQui == demotorzone && hpRefill == 1) {
            combattimentoFinito = 1;
            demotorzoneSconfitto = 1;
            printf(MAGENTA"\n[!!!] Il Demotorzone cade a terra di nuovo! E' la volta buona, ha smesso di respirare!\n"STOCK);
            printf("\nPremi un tasto per continuare\n");
            getch();
            return 1;
        }
        else if(nemicoQui == demotorzone && hpRefill == 0) {
            hpRefill = 1;
            printf(MAGENTA"\n[!!!] Non sembra vero, il Demotorzone è a terra senza forze. Ti assicuri che non respiri più...\n"STOCK);
                printf("\nPremi un tasto per continuare\n");
                getch();
            while(hpNemico != 21) {
                system("clear");
                printf(GIALLO"-- COMBATTIMENTO --\n"STOCK);
                printf("\nNemico: %s%s%s\n", ROSSO, nome_nemico(nemicoQui), STOCK);
                printf("\nHP Giocatore: %s%d%s\n",MAGENTA,p->hp,STOCK);
                printf("HP Nemico: %s%d%s\n",ROSSO, hpNemico, STOCK);
                hpNemico++;
                usleep(50000);
            }
            printf(ROSSO"\n[!] IL DEMOTORZONE SI E' RIALZATO!\n");
            printf("\nPremi un tasto per continuare\n");
            getch();
            continue;
        }
        else {
        printf(VERDE"\n\n[✓] Il nemico è stato sconfitto... per ora.\n"STOCK);
        combattimentoFinito = 1;
        printf("\nPremi un tasto per continuare\n");
        getch();
        return 1; // NEMICO SCONFITTO;
        }
    }
    }
}
    return -1;
 }

 
static int apriZaino(struct Giocatore* p) {
    int sceltaZaino = 0;
    int oggettoUsato = 0; // Flag per uscire dal loop

    while(!oggettoUsato) {
        // Aggiorniamo il menu ogni volta (perché gli oggetti cambiano)
        const char* menuApriZaino[] = {
            nome_oggetto(p->zaino[0]),
            nome_oggetto(p->zaino[1]),
            nome_oggetto(p->zaino[2]),
            "Indietro"
        };

        system("clear");
        printf(GIALLO"-- ZAINO --\n\n"STOCK);
        stampa_menu(menuApriZaino, 4, sceltaZaino);

        if(gestisci_navigazione(&sceltaZaino, 4) == 1) {
            
            // Caso 3: Indietro / Annulla
            if (sceltaZaino == 3) {
                return 0; // Esce dalla funzione senza fare nulla
            }

            // Recuperiamo l'oggetto selezionato
            Tipo_oggetto oggettoAttuale = p->zaino[sceltaZaino];

            switch(oggettoAttuale) {
                case nessun_oggetto:
                    printf(ROSSO"\n\n[!] Hai selezionato uno SLOT vuoto!\n" STOCK);
                    getch();
                    break; // Torna al while
                
                case bicicletta: 
                    printf(VERDE"\n\n[✓] Monti in sella alla tua bicicletta per scappare dal combattimento!\n"STOCK);
                    p->zaino[sceltaZaino] = nessun_oggetto;
                    oggettoUsato = 1;
                    printf("\nPremi un tasto per continuare...");
                    getch();
                    return 2;
                
                case maglietta_fuocoinferno:
                    p->hp = 20; 
                    printf(VERDE"\n\n[✓] Hai indossato la Maglietta Fuocoinferno. Ti senti rinvigorito! (HP Ripristinati)\n" STOCK);
                    p->zaino[sceltaZaino] = nessun_oggetto; // Consuma oggetto
                    oggettoUsato = 1;
                    printf("\nPremi un tasto per continuare...");
                    getch();
                    break;

                case bussola:
                    {
                        int posGiocatore = 1;
                        struct Zona_mondoreale* tempG = prima_zona_mondoreale;
                        while(tempG != NULL && tempG != p->pos_mondoreale) {
                            tempG = tempG->avanti;
                            posGiocatore++;
                        }

                        int posDemo = 1;
                        struct Zona_soprasotto* tempD = prima_zona_soprasotto;
                        while(tempD != NULL) {
                            if(tempD->nemico == demotorzone) break;
                            tempD = tempD->avanti;
                            posDemo++;
                        }

                        // Calcola la distanza
                        int distanza = posDemo - posGiocatore;
                        if(distanza < 0) distanza = -distanza; 

                        // Stampa
                        printf(VERDE"\n\n[✓] Tiri fuori la bussola mistica...\n"STOCK);
                        
                        if (distanza == 0) {
                            printf(ROSSO"    L'ago gira all'impazzata! IL DEMOTORZONE È QUI (nel Soprasotto)!\n"STOCK);
                        }
                        else {
                            // Direzione
                            char* direzione = (posDemo > posGiocatore) ? "AVANTI" : "INDIETRO";
                            
                            // Intensità basata sulla distanza
                            if (distanza <= 3) {
                                printf(GIALLO"    L'ago vibra violentemente puntando %s! Il segnale è fortissimo!\n"STOCK, direzione);
                                printf("    (Il Demotorzone è molto vicino...)\n");
                            } else {
                                printf("    L'ago punta stabilmente verso %s.\n", direzione);
                                printf("    (Il segnale è debole, la strada per il Demotorzone è ancora lunga.)\n");
                            }
                        }
                    }
                    getch();
                    break;

                case schitarrata_metallica:
                    printf(VERDE"\n\n[✓] Suoni un accordo potente! Senti il potere scorrere in te!\n"STOCK);
                    printf(GIALLO"    (Il tuo prossimo attacco sarà un CRITICO RADDOPPIATO!)\n"STOCK);
                    p->boost_critico = 1;
                    p->zaino[sceltaZaino] = nessun_oggetto;
                    oggettoUsato = 1;
                    getch();
                    break;
            }
        }
    }
    return 1;
}
    


void termina_gioco(){
    if (prima_zona_mondoreale != NULL) {
        cancella_mappa_esistente();
    }

    for(int i=0; i<4; i++) {
        if(player[i] != NULL) {
            free(player[i]);
            player[i] = NULL; }
    }

    gioco_impostato = 0;

    system("clear");
    printf(VERDE"\nGrazie per aver giocato a COSE STRANE!\n\n"STOCK);
}

// Funzione per controllare se un tasto è premuto (NON BLOCCANTE)


void crediti() {
    int kkk = 0;
    // Messaggio iniziale per avvisare l'utente
    system("clear");
    printf("Caricamento crediti...\n");
    
    while(1) {
        system("clear");
        printf(GIALLO"\t-- CREDITI --\n\n"STOCK);
        printf("Progetto: %sCose Strane%s\n", ROSSO, STOCK);
        
        // IL NOME CHE CAMBIA COLORE (Effetto Jeb_)
        printf("Creato da: %sLorenzo Ceccarelli%s\n", allColors[kkk], STOCK); 

        printf(CIANO"\n--- ALBO D'ORO (Ultimi Vincitori) ---\n"STOCK);
        for(int i=0; i<3; i++) {
            if(strcmp(storicoVincitori[i], "") != 0) {
                printf("%d. %s\n", i+1, storicoVincitori[i]);
            } else {
                printf("%d. ---\n", i+1);
            }
        }
        
        printf("\n\nPremi un tasto per tornare al menù...");
        fflush(stdout); // Forza la stampa immediata
        
        // Animazione
        usleep(75000); // 0.075 secondi di pausa
        
        // Cambio colore
        kkk++;
        if(kkk >= 20) {
            kkk=0;
        }

        // --- LA MAGIA: CONTROLLO NON BLOCCANTE ---
        if(getchNoWait()) {
            break; // Se hai premuto un tasto, esci dal while
        }
    }
}


void regole() {
    system("clear");
    printf(GIALLO "\n   --- MANUALE DI SOPRAVVIVENZA A OCCHINZ ---\n\n" STOCK);

    printf(CIANO "1. OBIETTIVO DEL GIOCO\n" STOCK);
    printf("   Sei intrappolato nella strana cittadina di Occhinz. Il tuo obiettivo è sopravvivere\n");
    printf("   il più a lungo possibile contro le forze dell'oscurità e sconfiggere il DEMOTORZONE.\n");
    printf("   Se tutti i giocatori muoiono, il gioco termina con una sconfitta.\n\n");

    printf(CIANO "2. IL MONDO E IL SOPRASOTTO\n" STOCK);
    printf("   La mappa è divisa in due dimensioni parallele:\n");
    printf("   - " VERDE "Mondo Reale:" STOCK " Più sicuro, qui puoi trovare oggetti preziosi.\n");
    printf("   - " ROSSO "Soprasotto:" STOCK " Un luogo oscuro. Qui non ci sono oggetti, ma mostri terribili.\n");
    printf("     Fai attenzione: Entrare nel Soprasotto è facile, ma per USCIRE devi lanciare\n");
    printf("     un dado: se il risultato è inferiore alla tua Fortuna, il portale si apre.\n");
    printf("     Altrimenti... resti intrappolato!\n\n");

    printf(CIANO "3. MOVIMENTO\n" STOCK);
    printf("   - " GIALLO "Avanza:" STOCK " Ti sposti alla zona successiva.\n");
    printf("   - " GIALLO "Indietreggia:" STOCK " Torni alla zona precedente.\n");
    printf("   - " GIALLO "Cambia Mondo:" STOCK " Passi all'altra dimensione (vedi regole sopra).\n");
    printf("   Ricorda: Non puoi cambiare zona se c'è un nemico vivo nella stanza!\n\n");

    printf(CIANO "4. COMBATTIMENTO\n" STOCK);
    printf("   Quando incontri un nemico, inizia il combattimento a turni.\n");
    printf("   - Attacco = (Attacco Psichico + d20 + Fortuna/4).\n");
    printf("   - " GIALLO "Colpo Critico:" STOCK " Se il dado è >= 17, infliggi danni massicci!\n");
    printf("   - " ROSSO "Fallimento:" STOCK " Se il dado è < 8, il tuo attacco va a vuoto.\n");
    printf("   - Dopo il tuo turno, il nemico contrattacca sulla tua Difesa Psichica.\n");
    printf("   - Se vinci, c'è il " VERDE "50%% di probabilità" STOCK " che il nemico scompaia per sempre.\n\n");

    printf(CIANO "5. GLI OGGETTI (ZAINO)\n" STOCK);
    printf("   Puoi portare fino a 3 oggetti nello zaino. Ecco a cosa servono:\n");
    printf("   - " MAGENTA "Bicicletta:" STOCK " Essenziale per SCAPPARE dai combattimenti senza subire danni.\n");
    printf("     Si usa dal menu di combattimento -> Scappa.\n");
    printf("   - " MAGENTA "Maglietta Fuocoinferno:" STOCK " Ti cura completamente (riporta gli HP a 20).\n");
    printf("   - " MAGENTA "Schitarrata Metallica:" STOCK " Potenzia il tuo spirito: il tuo prossimo attacco\n");
    printf("     sarà un COLPO CRITICO RADDOPPIATO devastante.\n");
    printf("   - " MAGENTA "Bussola:" STOCK " Punta sempre alla zona dove si nasconde il Demotorzone.\n\n");

    printf(CIANO "6. STATISTICHE\n" STOCK);
    printf("   - " BLU "HP (Salute):" STOCK " Se scende a 0, il giocatore muore definitivamente.\n");
    printf("   - " BLU "Attacco Psichico:" STOCK " Determina il danno inflitto.\n");
    printf("   - " BLU "Difesa Psichica:" STOCK " Riduce il danno subito dai nemici.\n");
    printf("   - " BLU "Fortuna:" STOCK " Aumenta la probabilità di colpire e serve per fuggire dal Soprasotto.\n\n");

    printf("\n   Premi un tasto per tornare al menu...\n");
    getch();
}