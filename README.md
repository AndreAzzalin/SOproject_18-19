# SOproject
Creating a little kernel of a operating system, with C language, that synchronize with each other processes through IPC System V.
This is my university project, i hope it will be useful!
In the coming days i will add more details and some information of the code & variables, so stay tuned :P

RELAZIONE PROGETTO SISTEMI OPERATIVI  2018/2019
Per scrivere il codice sorgente per il progetto di SO ho creato un file d’intestazione “libs_struct.h” per includere librerie, per la definizione di qualche macro e dichiarazione delle variabili che userò nel progetto e dichiaro così una volta per tutte le librerie che mi sono necessarie.
Poi ho iniziato a pensare ed a scrivere codice per il processo professore, che una volta entrato in esecuzione, allocherà le strutture necessarie per il progetto: memoria condivisa, semafori e coda di messaggi.
Creerà una memoria condivisa: al cui interno gli studenti scriveranno le proprie informazioni per essere contattati per entrare in gruppo di studenti e una volta accordati iscriveranno il proprio gruppo nella memoria condivisa per essere poi valutato.
Creerà POP_SIZE processi figli con l’uso di una fork annidata all’interno di un ciclo for: una volta che il processo figlio è stato creato viene soprascritto il codice eseguibile con il codice binario dello studente.
Avviato un timer di n sec per la simulazione, passati tali secondi farà terminare l’esecuzione dei processi figli ed effettuerà i calcoli opportuni per decretare l’esito dell’esame.

Il processo studente, legge da file.config i vari parametri per l’esecuzione della prova, ed aspetterà che tutti gli altri compagni siano stati inizializzati a dovere attraverso l’uso di un set di semafori. 
Una volta che tutti sono stati creati, iniziano la ricerca dei possibili compagni con l’aiuto della memoria condivisa e lo scambio degli inviti attraverso una coda di messaggi per gli studenti facenti parte del turno pari e un’altra coda di messaggi per gli studenti del turno dispari.
Quando è che il tempo è scaduto, gli studenti riceveranno un SIGINT per segnalare che devono terminare la loro esecuzione, però prima di ciò devono stampare a video il voto finale ottenuto.

Descrizione uso IPC:
Ho creato un set da 2 semafori: il primo mi servirà come semaforo contatore e l’altro per la mutua esclusione delle risorse.
Utilizzo 3 code di messaggi: mid0 coda di messaggi per gli studenti pari, mid1 coda di messaggi per gli studenti dispari e msgid è usata per comunicare con il professore.
Alloco una memoria condivisa: metà popolata con le informazioni riguardanti gli studenti e l’altra metà con le informazioni dei gruppi.
