#include <stdio.h>
#include <stdlib.h>
#include "gamelib.h"
#include <termios.h>
#include <unistd.h>

int main(void) { 
    int scelta = 0;

        do {
        system("clear");
        printf(ROSSO"**********************************");
        printf("\n\t--- COSESTRANE ---\n"); 
        printf("**********************************\n"STOCK);

        const char* opzioni_menu_principale[] = {
            "Imposta Gioco",
            "Gioca",
            "Regole",
            "Termina Gioco",
            "Visualizza Crediti",
        };

        stampa_menu(opzioni_menu_principale, 5, scelta);
        if(gestisci_navigazione(&scelta, 5) == 1) {
            switch(scelta) {
                case 0: imposta_gioco(); break;
                case 1: gioca(); break;
                case 2: regole(); break;
                case 3: termina_gioco(); return 0; 
                case 4: crediti(); break; 
                }
            }
        } while(1);
    }   


// A = UP
// B = DOWN
// C = RIGHT 
// D = LEFT
