#include "lib.h"


int my_nof_elems;
int my_voto_AdE;
int my_libero;
int nof_invites;

int my_msg_queue;

int main(int argc, char *argv[]) {


    init();
    TEST_ERROR

    index_POPSIZE = 0;

    while (1) {

        //uso il sem0 per capire quando tutti i processi sono stati caricati
        if (getSemVal(0) == 0) {


            //prima rispondo a tutti e poi invio msg
            while (msgrcv(my_msg_queue, &msg_queue, sizeof(msg_queue) - sizeof(long), getpid(), IPC_NOWAIT) ==
                   sizeof(msg_queue) - sizeof(long)) {

                /*    switch (msg_queue.oggetto) {

                        case INVITO:

                            reserveSem(1);
                            int nof_invites_reply = shdata_pointer->students[INDEX].nof_invites_reply;
                            int nof_invites_send = shdata_pointer->students[INDEX].nof_invites_send;
                            int io = shdata_pointer->students[INDEX].voto_AdE;
                            int mitt_voto = shdata_pointer->students[INDEX_MITT].voto_AdE;
                            int mitt = shdata_pointer->students[INDEX_MITT].matricola;
                            releaseSem(1);

                            if (nof_invites_reply + nof_invites_send == 4) {

                                if (io <= mitt_voto) {
                                    //    printf("[%d] ACCETTO [%d]\n", getpid(), mitt);
                                    accept();
                                } else {
                                    reserveSem(1);
                                    shdata_pointer->students[INDEX_MITT].nof_invites_reply++;
                                    releaseSem(1);
                                    refuse();
                                }

                            } else {
                                // printf("[%d] RIFIUTO PER REPLY [%d]\n", getpid(), mitt);
                                reserveSem(1);
                                shdata_pointer->students[INDEX_MITT].nof_invites_reply++;
                                releaseSem(1);
                                break;
                            }

                            break;


                        case REPLY:
                            reserveSem(1);

                            // printf("[%d] REPLY aggiungere al gruppo (%d)\n", getpid(), msg_queue.student_mitt);
                            //caso 1.1 : gruppo non esiste e sono entrambi liberi
                            if (shdata_pointer->students[INDEX].libero == TRUE &&
                                shdata_pointer->students[INDEX_MITT].libero == TRUE &&
                                shdata_pointer->groups[INDEX].capo != getpid()) {


                                shdata_pointer->students[INDEX].libero = FALSE;
                                shdata_pointer->students[INDEX_MITT].libero = FALSE;

                                //creo gruppo e divento capo
                                shdata_pointer->groups[INDEX].capo = getpid();
                                shdata_pointer->groups[INDEX].chiuso = FALSE;

                                //inserisco primo compagni che mi ha risposto
                                shdata_pointer->groups[INDEX].compagni[1] = msg_queue.student_mitt;
                                shdata_pointer->groups[INDEX].compagni[0] = getpid();

                                //printf("[%d] INSERISCO [%d]\n", getpid(), msg_queue.student_mitt);

                                //caso 1.2 : non sono libero e sono capo di un gruppo
                            } else {


                                if (shdata_pointer->groups[INDEX].capo == getpid() &&
                                    shdata_pointer->students[INDEX].libero == FALSE &&
                                    shdata_pointer->groups[INDEX].chiuso == FALSE) {

                                    //printf("[%d] sono capo e voglio inserire %d\n", getpid(), msg_queue.student_mitt);
                                    //inserisco studente alla lista compagni
                                    for (int i = 2; i < 4; ++i) {

                                        //  printf("[%d] INSERISCO else [%d]\n", getpid(), msg_queue.student_mitt);

                                        if (shdata_pointer->groups[INDEX].compagni[i] == 0 &&
                                            shdata_pointer->students[INDEX_MITT].libero) {


                                            shdata_pointer->groups[INDEX].compagni[i] = msg_queue.student_mitt;
                                            shdata_pointer->students[INDEX_MITT].libero = FALSE;
                                        }
                                    }

                                    //se il gruppo raggiunge nof_elems imposto a chiuso
                                    if (shdata_pointer->groups[INDEX].compagni[shdata_pointer->students[INDEX].nof_elems -
                                                                               1] !=
                                        0) {
                                        shdata_pointer->groups[INDEX].chiuso = TRUE;
                                    }
                                }
                            }
                            releaseSem(1);

                            break;
                    }*/



                switch (msg_queue.oggetto) {


                    case REPLY:
                        reserveSem(1);



                        /*
                         *  qualcuno ha risposto positivamente a un invito
                         * è il capo del gruppo a inserire i componenti al prorpio gruppo
                                            */


                        if (SH_INDEX.libero && SH_MITT.libero) {

                            SH_INDEX.libero = FALSE;
                            SH_MITT.libero = FALSE;


                            int x = SH_MITT.matricola;
                            int y = getpid();

                            G_INDEX.compagni[0] = getpid();
                            G_INDEX.compagni[1] = x;


                            SH_INDEX.libero = FALSE;
                            SH_MITT.libero = FALSE;

                            // printf("[%d] nof %d chiuso? %d\n",getpid(),SH_INDEX.nof_elems,G_INDEX.chiuso);

                            if (SH_INDEX.nof_elems == 2) {
                                G_INDEX.chiuso = TRUE;
                            }


                        } else if (G_INDEX.compagni[0] == getpid() && G_INDEX.chiuso == FALSE) {
                            /*
                             * sono capo e devo chiudere il gruppo
                             * rimane caso gruppi formati da 3 o 4 studenti
                             */

                            SH_MITT.libero = FALSE;

                            if (SH_INDEX.nof_elems == 3) {
                                G_INDEX.compagni[2] = msg_queue.student_mitt;
                                G_INDEX.chiuso = TRUE;

                            } else if (SH_INDEX.nof_elems == 4) {
                                G_INDEX.compagni[3] = msg_queue.student_mitt;
                                G_INDEX.chiuso = TRUE;
                            }

                        }


                        releaseSem(1);

                        break;


                    case INVITO:

                        reserveSem(1);

                        /*
                         * se non ho ricevuto tutte le risposte invio un WAIT
                         * se ricevo un wait è come se rifiutassi ma senza reject--
                         *
                         */

                        int flag_wait = 0;
                        for (int j = 0; j < nof_invites; ++j) {
                            if (SH_INDEX.utils[j].pid_invitato > 0 && SH_INDEX.utils[j].reply == FALSE) {
                                flag_wait = -1;
                                break;
                            }
                        }


                        if (flag_wait < 0) {
                            /*
                             * se non ho ricevuto risposta da tutti invio un wait al mittente
                             */
                            SET_REPLY_TRUE

                        } else if (((SH_INDEX.voto_AdE - SH_MITT.voto_AdE) < 5 && SH_INDEX.libero) ||
                                   SH_INDEX.nof_reject < 0) {
                            /*
                             * accetta se compatibile o se ha finito i reject
                             * viene impostato il mitta true
                             * viene inserito nel gruppo
                             * se è il primo
                             */
                            SET_REPLY_TRUE

                            msg_queue.mtype = msg_queue.student_mitt;
                            msg_queue.student_mitt = getpid();
                            msg_queue.oggetto = REPLY;

                            if (msgsnd(my_msg_queue, &msg_queue, sizeof(msg_queue) - sizeof(long), 0) < 0)
                                TEST_ERROR


                        } else {
                            /*
                             * rifiuto per incompatibilità
                             */
                            SET_REPLY_TRUE
                            SH_INDEX.nof_reject--;

                        }

                        releaseSem(1);

                        break;
                }
            }


            /*
             * sezione invio msg
             */

            reserveSem(1);

            /*
             * se ho inviti disponibili
             * se sono libero
             * se sono capo e non ho ancora chiuso il gruppo
             */
            if ((SH_INDEX.nof_invites_send > 0 && SH_INDEX.libero) ||
                (G_INDEX.capo == getpid() && G_INDEX.chiuso == FALSE)) {

                //cerco studenti compatibili
                if (checkPariDispari(SH_TO_INVITE.matricola) && (SH_INDEX.matricola != SH_TO_INVITE.matricola) &&
                    (SH_INDEX.voto_AdE <= SH_TO_INVITE.voto_AdE) &&
                    (SH_INDEX.nof_elems == SH_TO_INVITE.nof_elems) && SH_TO_INVITE.libero) {


                    //controllo a quali studenti ho giù chiesto
                    int flag_no_spam = TRUE;
                    for (int i = 0; i < 4; ++i) {
                        if (SH_INDEX.utils[i].pid_invitato == SH_TO_INVITE.matricola) {
                            flag_no_spam = FALSE;
                        }
                    }

                    /*
                     * se non ho mai invitato aggiungo alla lista di invitati
                     */
                    if (flag_no_spam) {

                        SH_INDEX.utils[SH_INDEX.nof_invites_send - 1].pid_invitato = SH_TO_INVITE.matricola;
                        SH_INDEX.utils[SH_INDEX.nof_invites_send - 1].reply = FALSE;

                        SH_INDEX.nof_invites_send--;


                        msg_queue.mtype = SH_TO_INVITE.matricola;
                        msg_queue.student_mitt = getpid();
                        msg_queue.oggetto = INVITO;

                        //printf("[%d] | dest %ld | mitt %d\n", getpid(), msg_queue.mtype, msg_queue.student_mitt);

                        if (msgsnd(my_msg_queue, &msg_queue, sizeof(msg_queue) - sizeof(long), 0) < 0)
                            TEST_ERROR
                    }
                }
            }

            releaseSem(1);


            index_POPSIZE++;

            if (index_POPSIZE == POP_SIZE) {
                index_POPSIZE = 0;
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

    sem_id = semget(key, 2, IPC_CREAT | 0666);

    TEST_ERROR


    //inizializzo le variabili che caratterizzano uno studente
    reserveSem(1);

    printf("\n=== STUDENTE[%d] ===\n", getpid());

    printf("semid = %d key =  %d\n", sem_id, key);

    my_nof_elems = getNof_elems();
    my_voto_AdE = getVoto();

    nof_invites = shdata_pointer->config_values[3];


    shdata_pointer->students[INDEX].matricola = getpid();
    shdata_pointer->students[INDEX].nof_elems = my_nof_elems;
    shdata_pointer->students[INDEX].voto_AdE = my_voto_AdE;
    shdata_pointer->students[INDEX].libero = TRUE;

    shdata_pointer->students[INDEX].nof_invites_send = nof_invites;
    shdata_pointer->students[INDEX].nof_reject = shdata_pointer->config_values[4];

    for (int i = 0; i < nof_invites; ++i) {
        shdata_pointer->students[INDEX].utils[i].pid_invitato = -1;
        shdata_pointer->students[INDEX].utils[i].reply = FALSE;
    }

    for (int j = 0; j < POP_SIZE; ++j) {
        G_INDEX.capo = -1;
        G_INDEX.chiuso = FALSE;
        for (int i = 0; i < 4; ++i) {
            G_INDEX.compagni[i] = -1;
        }
    }


    printf("my_nofelems= %d\n", my_nof_elems);
    printf("my_voto= %d\n", my_voto_AdE);
    printf("my_nof_send= %d\n", shdata_pointer->students[INDEX].nof_invites_send);


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


void search_colleagues(int my_nof_elems, int my_voto_AdE, int my_matricola) {

    pid_t colleague_found = -1;

    reserveSem(1);
    for (int i = 0; i < POP_SIZE; ++i) {
        //se matricola corrisponde al turno corretto e non mi invio msg da solo
        if (checkPariDispari(shdata_pointer->students[i].matricola)) {
            if (shdata_pointer->students[i].libero && shdata_pointer->students[i].matricola != getpid()) {
                //se abbiamo lo stesso nof invia
                if (shdata_pointer->students[i].nof_elems >= my_nof_elems) {
                    colleague_found = shdata_pointer->students[i].matricola;
                    //se ha un voto maggiore o uguale al mio
                    if (shdata_pointer->students[i].voto_AdE >= my_voto_AdE) {
                        colleague_found = shdata_pointer->students[i].matricola;
                        break;
                    }
                    break;
                }
            } else {
                //nessuno compatibile
                colleague_found = -1;
            }
        }
    }


    releaseSem(1);


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

    /*
     * invio messaggio ti tipo accept
     */

    msg_queue.mtype = msg_queue.student_mitt;
    msg_queue.student_mitt = getpid();
    msg_queue.oggetto = REPLY;


    if (msgsnd(my_msg_queue, &msg_queue, sizeof(msg_queue) - sizeof(long), 0) < 0) {
        TEST_ERROR
    }
}

void refuse() {
    reserveSem(1);
    int nof_reject = shdata_pointer->students[INDEX].nof_reject;
    releaseSem(1);

    if (nof_reject > 0) {
        //printf("[%d] RIFIUTO %d | reject %d \n", getpid(), msg_queue.student_mitt, nof_reject);

        reserveSem(1);
        shdata_pointer->students[INDEX].nof_reject--;
        releaseSem(1);

    } else {
        //printf("[%d] ACCETTO %d PER REJECT TERMINATI\n", getpid(), msg_queue.student_mitt);
        accept();
    }
}


