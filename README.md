🚲 Cose Strane - Text-Based RPG in C
Benvenuto a Occhinz, una tranquilla cittadina di provincia che nasconde un terribile segreto. Cose Strane è un gioco di ruolo testuale (RPG) sviluppato in C, in cui un gruppo di coraggiosi ragazzi deve esplorare la città, viaggiare tra dimensioni parallele e sopravvivere agli orrori del Soprasotto per sconfiggere il temibile Demotorzone.

📖 Come Funziona il Gioco (Regole)
Il gioco è strutturato a turni. Da 1 a 4 giocatori esplorano una mappa generata casualmente (o creata manualmente dal Game Master) composta da 15 zone.

🌌 Le Due Dimensioni
Ogni zona della mappa esiste in due versioni sovrapposte:

Mondo Reale: È il luogo dove puoi trovare oggetti utili per la sopravvivenza. Anche qui ci sono dei nemici, ma l'ambiente è meno ostile.

Soprasotto: Una dimensione oscura e fredda. Qui non ci sono oggetti, ma si nascondono i mostri più letali. Entrare nel Soprasotto tramite un portale è facile, ma per uscirne dovrai lanciare un dado: solo se il tiro è inferiore alla tua statistica Fortuna, il portale si aprirà.

⚔️ Combattimento e Statistiche
Quando incontri un nemico (Billi, Democane o Demotorzone), non puoi avanzare finché non lo sconfiggi o non scappi. Il combattimento è ispirato ai giochi di ruolo da tavolo (D&D):

Il Lancio (Roll): L'attacco è calcolato lanciando un dado da 20 facce (d20) e sommandolo a un bonus derivato dalla tua Fortuna.

Colpi Critici & Fallimenti: Se il roll è basso (< 8), l'attacco va a vuoto. Se è molto alto (>= 17), infliggi danni Critici.

Difesa: Al termine del tuo turno, il nemico contrattacca. Il danno subito dipenderà dalla tua statistica di Difesa Psichica.

Se i tuoi HP (Salute) scendono a 0, il tuo personaggio muore definitivamente (Permadeath). Se tutti i giocatori muoiono, è Game Over.

🎒 Lo Zaino e gli Oggetti
Ogni giocatore ha uno zaino con 3 slot. Nel Mondo Reale è possibile trovare:

🚲 Bicicletta: Permette di fuggire istantaneamente da un combattimento.

👕 Maglietta Fuocoinferno: Ripristina completamente la salute (20 HP).

🎸 Schitarrata Metallica: Ti carica di adrenalina. Il tuo prossimo attacco sarà un Colpo Critico Raddoppiato garantito.

🧭 Bussola: Un oggetto mistico che ti indica la distanza e la direzione in cui si nasconde il boss finale (Demotorzone).

🚀 Compilazione ed Esecuzione
Il progetto è scritto interamente in C standard. Per giocare, è necessario compilare i file sorgente utilizzando il compilatore GCC (su Linux, macOS o WSL per Windows).

Apri il terminale, posizionati nella cartella contenente i file del gioco ed esegui questi due comandi:

1. Compilazione:

Bash

gcc main.c gamelib.c -o gioco
2. Esecuzione:

Bash

./gioco
🛠️ Commenti e Modifiche rispetto alla Traccia Originale
Durante lo sviluppo del progetto, ho deciso di implementare alcune modifiche e migliorie tecniche rispetto alle specifiche di base della traccia, con l'obiettivo di rendere l'esperienza di gioco molto più interattiva e curata.

Navigazione Dinamica dei Menu: Invece di utilizzare i classici inserimenti numerici bloccanti tramite scanf, ho implementato la navigazione dei menu tramite le freccette direzionali della tastiera, confermando le scelte con il tasto Invio.

Interfaccia Grafica e Codici Colore (ANSI): Ho introdotto i codici colore ANSI per migliorare drasticamente la leggibilità. I messaggi di sistema, gli errori e i successi sono colorati semanticamente (es. Rosso per gli allarmi, Verde per i successi), e ad ogni giocatore è assegnato dinamicamente un colore univoco ad inizio partita.

Sistema di Combattimento Ispirato agli RPG: Il combattimento non si basa su semplici sottrazioni matematiche statiche, ma su un sistema randomico in cui la statistica Fortuna gioca un ruolo cruciale, determinando la probabilità di colpo critico o di fallimento.

Classe Segreta "UndiciVirgolaCinque": Aggiunta la possibilità (limitata a un solo giocatore per partita) di trasformarsi nella classe "UndiciVirgolaCinque", barattando quasi tutta la propria Fortuna per ottenere ingenti bonus su Attacco e Difesa.

Morte Definitiva (Permadeath) e Game Over: Implementata la gestione avanzata dei round. Se un giocatore perde tutti gli HP, salterà automaticamente i turni successivi. Se l'intero party viene sconfitto, la partita termina.

🔓 Funzioni Pubbliche e Aggiunte al Progetto
Oltre alle quattro funzioni standard richieste dalla traccia (imposta_gioco, gioca, termina_gioco, crediti), ho deciso di non limitare l'accesso (omettendo la keyword static) ad alcune funzioni di utilità che ritengo fondamentali per la modularità del progetto:

getch() e getchNoWait(): Funzioni custom per acquisire l'input da tastiera (disabilitando l'ECHO del terminale). getchNoWait() in particolare mi ha permesso di creare un'animazione continua e non bloccante per la schermata dei Crediti (comportamento altrimenti impossibile con le funzioni di input standard della libreria C).

stampa_menu() e gestisci_navigazione(): Il "motore" grafico del gioco. Rendendole pubbliche, il sistema di menu a scelta multipla diventa un modulo riutilizzabile globalmente.

regole() (NUOVA FEATURE): Ho implementato ex-novo questa funzione pubblica, richiamabile dal menu principale in main.c, che funge da "Manuale di Sopravvivenza" in-game per spiegare ai giocatori le meccaniche modificate.

Autore: Lorenzo Ceccarelli

Progetto Finale per il corso di Programmazione Procedurale.
