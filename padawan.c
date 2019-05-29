#include "lib.h"


int my_nof_elems;
int my_voto_AdE;
int my_libero;

int nof_invites;
int nof_reject;

int my_msg_queue;

int main(int argc, char *argv[]) {


    init();
    TEST_ERROR


    while (1) {

        //uso il sem0 per capire quando tutti i processi sono stati caricati
        if (getSemVal(0) == 0) {


            //prima rispondo e poi invio msg
            while (msgrcv(my_msg_queue, &msg_queue, sizeof(msg_queue) - sizeof(long), getpid(), IPC_NOWAIT) ==
                   sizeof(msg_queue) - sizeof(long)) {

                switch (msg_queue.oggetto) {

                    //caso 2 : ricevo invito
                    case INVITO:
                     //   printf("[%d] RICEVUTO msg tipo INVITO da (%d)\n", getpid(), msg_queue.student_mitt);

                        //controllo se compatibile (double check)
                        reserveSem(1);
                        int var = shdata_pointer->students[msg_queue.student_mitt % POP_SIZE].voto_AdE;
                        int var2 = shdata_pointer->students[msg_queue.student_mitt % POP_SIZE].matricola;
                        releaseSem(1);

                        if (var >= my_voto_AdE && var2 != getpid()) {

                            //caso 2.1: se compatibile accetto
                            accept();

                            //caso 2.1: se non compatibile rifiuto
                        } else {
                            //caso 2.2 : non compatibile ma nof_reject finiti
                            refuse();

                        }


                        break;


                        //caso 1 : ricevo risposta a un mio msg
                    case REPLY:
                        reserveSem(1);
/*
                        //caso 1.1 gruppo non esiste -> crealo

                        my_libero = FALSE;

                        //capo gruppo
                        if (shdata_pointer->students[msg_queue.mtype % POP_SIZE].matricola == msg_queue.mtype) {

                            shdata_pointer->students[msg_queue.mtype % POP_SIZE].libero = FALSE;

                            //creo gruppo
                            if (shdata_pointer->groups[INDEX].compagni[0] == 0) {
                                shdata_pointer->groups[INDEX].capo = getpid();
                                shdata_pointer->groups[INDEX].compagni[0] = getpid();
                                shdata_pointer->groups[INDEX].chiuso = FALSE;
                            } else {
                                //caso 1.2 gruppo già esistente -> capo inserisce studente nel gruppo
                                printf("capo[%d] to add [%d]\n",getpid(),msg_queue.student_mitt);
                                if (shdata_pointer->students[msg_queue.student_mitt % POP_SIZE].matricola ==
                                    msg_queue.student_mitt) {

                                    shdata_pointer->students[msg_queue.student_mitt % POP_SIZE].libero = FALSE;

                                    //inserisco studente nel gruppo
                                    for (int i = 0; i < 4; ++i) {
                                        if (shdata_pointer->groups[INDEX].compagni[i] == 0) {
                                            shdata_pointer->groups[INDEX].compagni[i] == msg_queue.student_mitt;
                                        }
                                    }

                                    //se il gruppo è pieno lo chiudo
                                    if (shdata_pointer->groups[INDEX].compagni[3]) {
                                        shdata_pointer->groups[INDEX].chiuso = TRUE;
                                    }
                                }
                            }
                        }

                        //studente che ha accettato
                        //printf("capo[%d] RICEVUTO msg tipo REPLY da (%d)\n", getpid(), msg_queue.student_mitt);


*/
                       // printf("[%d] REPLY aggiungere al gruppo (%d)\n", getpid(), msg_queue.student_mitt);
                        //caso 1.1 : gruppo non esiste e sono entrambi liberi
                        if (shdata_pointer->students[INDEX].libero &&
                            shdata_pointer->students[INDEX_MITT].libero &&
                            shdata_pointer->groups[INDEX].capo != getpid()) {

                            shdata_pointer->students[INDEX].libero = FALSE;


                            //creo gruppo e divento capo
                            shdata_pointer->groups[INDEX].capo = getpid();
                            shdata_pointer->groups[INDEX].compagni[0] = getpid();
                            shdata_pointer->groups[INDEX].chiuso = FALSE;

                            //inserisco primo compagni che mi ha risposto
                            shdata_pointer->groups[INDEX].compagni[1] = msg_queue.student_mitt;
                            shdata_pointer->students[INDEX_MITT].libero = FALSE;

                            //caso 1.2 : non sono libero e sono capo di un gruppo
                        } else if (shdata_pointer->groups[INDEX].capo == getpid() &&
                                   shdata_pointer->groups[INDEX].chiuso == FALSE) {

                            //printf("[%d] sono capo e voglio inserire %d\n", getpid(), msg_queue.student_mitt);
                            //inserisco studente alla lista compagni
                            for (int i = 2; i < 4; ++i) {
                                if (shdata_pointer->groups[INDEX].compagni[i] == 0 &&
                                    shdata_pointer->students[INDEX_MITT].libero) {

                                 //   printf("[%d] sono capo e voglio inserire %d\n", getpid(), msg_queue.student_mitt);

                                    shdata_pointer->groups[INDEX].compagni[i] = msg_queue.student_mitt;
                                    shdata_pointer->students[INDEX_MITT].libero = FALSE;
                                }
                            }

                            //se il gruppo raggiunge nof_elems imposto a chiuso
                            if (shdata_pointer->groups[INDEX].compagni[shdata_pointer->students[INDEX].nof_elems - 1] !=
                                0) {
                                shdata_pointer->groups[INDEX].chiuso = TRUE;
                            }
                        }

                        //caso 1.3 : non sono libero e non sono capo del gruppo -> non fai nulla
                        releaseSem(1);
                        break;

                }
            }



            //cerco destinatario
            msg_queue.mtype = search_colleagues(my_nof_elems, my_voto_AdE, getpid());

            //mittente
            msg_queue.student_mitt = getpid();

            msg_queue.oggetto = INVITO;


            //se ho nof_invites e sono libero invito compagni
            if (nof_invites > 0 && msg_queue.mtype != -1 && shdata_pointer->students[INDEX].libero == TRUE) {
                if (msgsnd(my_msg_queue, &msg_queue, sizeof(msg_queue) - sizeof(long), 0) < 0) {
                    TEST_ERROR
                } else {
                    nof_invites--;
                }
            }
        }
    }
}


void init() {


    //per random
    srand(getpid());

    //mi allaccio alle strutture IPC
    key = setKey();
    sem_id = semget(key, 0, IPC_CREAT);

    shmem_id = shmget(key, sizeof(struct shdata), IPC_CREAT | 0666);
    shdata_pointer = (struct shdata *) shmat(shmem_id, NULL, 0);


    msg_id = msgget(ID_KEY, IPC_CREAT | 0666);

    my_msg_queue = getMsgQueue();

    TEST_ERROR


    //inizializzo le variabili che caratterizzano uno studente
    reserveSem(1);

    printf("\n=== STUDENTE[%d] ===\n", getpid());

    printf("semid = %d key =  %d\n", sem_id, key);

    my_nof_elems = getNof_elems();
    my_voto_AdE = getVoto();
    my_libero = TRUE;

    nof_invites = shdata_pointer->config_values[3];
    nof_reject = shdata_pointer->config_values[4];


    shdata_pointer->students[INDEX].matricola = getpid();
    shdata_pointer->students[INDEX].nof_elems = my_nof_elems;
    shdata_pointer->students[INDEX].voto_AdE = my_voto_AdE;
    shdata_pointer->students[INDEX].libero = TRUE;


    printf("my_nofelems= %d\n", my_nof_elems);
    printf("my_voto= %d\n", my_voto_AdE);


    reserveSem(0);
    releaseSem(1);

    //punto alla funzione che gestirà il segnale
    sa.sa_handler = &signal_handler;
    //installo handler e controllo errore
    sigaction(SIGINT, &sa, &sa_old);
    TEST_ERROR

}

void signal_handler(int signalVal) {
    if (signalVal == SIGINT) {

        //stacco frammento di memoria da processo
        shmdt(shdata_pointer);

        msgctl(my_msg_queue, IPC_RMID, NULL);

        msgctl(msg_id, IPC_RMID, NULL);

        shmctl(shmem_id, IPC_RMID, NULL);


        exit(EXIT_SUCCESS);
    }
}

pid_t search_colleagues(int my_nof_elems, int my_voto_AdE, int my_matricola) {

    pid_t colleague_found = -1;

    reserveSem(1);
    for (int i = 0; i < POP_SIZE; ++i) {
        //se matricola corrisponde al turno corretto e non mi invio msg da solo
        if (checkPariDispari(shdata_pointer->students[i].matricola)) {
            if (shdata_pointer->students[i].libero && shdata_pointer->students[i].matricola != getpid()) {
                //se abbiamo lo stesso nof invia
                if (shdata_pointer->students[i].nof_elems == my_nof_elems) {
                    colleague_found = shdata_pointer->students[i].matricola;
                    //se ha un voto maggiore o uguale al mio
                    if (shdata_pointer->students[i].voto_AdE >= my_voto_AdE) {
                        colleague_found = shdata_pointer->students[i].matricola;
                        break;
                    }
                    break;
                }
            } else {
                colleague_found = -1;
            }
        }
    }

    releaseSem(1);

    return colleague_found;
}

int checkPariDispari(int matricola_to_compare) {
    if ((PARI && matricola_to_compare % 2 == 0) || (DISPARI && matricola_to_compare % 2 != 0)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int getMsgQueue() {
    if (PARI) {
        return msgget(KEY_PARI, IPC_CREAT | 0666);
    } else {
        return msgget(KEY_DISPARI, IPC_CREAT | 0666);
    }
}

void accept() {

    toReply_dest = msg_queue.student_mitt;
    msg_queue.mtype = toReply_dest;
    msg_queue.student_mitt = getpid();
    msg_queue.oggetto = REPLY;

    printf("[%d] ACCETTO | to %d | invites %d | index %d\n ", getpid(), toReply_dest,
           nof_invites, INDEX);

    if (msgsnd(my_msg_queue, &msg_queue, sizeof(msg_queue) - sizeof(long), 0) < 0) {
        TEST_ERROR
    }
}

void refuse() {
    if (nof_reject > 0) {
        printf("sono [%d] RIFIUTO %d | reject %d \n", getpid(), msg_queue.student_mitt, nof_reject);
        nof_reject--;
    } else {
        printf("sono [%d] ACCETTO %d PER REJECT TERMINATI\n", getpid(), msg_queue.student_mitt);
        accept();
    }
}




