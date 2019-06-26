#include "lib.h"


int my_nof_elems;
int my_voto_AdE;
int nof_invites;

int my_msg_queue;
int flag_no_spam = TRUE;
int flag_wait = 0;
int index_POPSIZE = 0;


int main(int argc, char *argv[]) {


    init();
    TEST_ERROR


    while (TRUE) {
        //uso il sem0 per capire quando tutti i processi sono stati caricati
        if (getSemVal(0) == 0) {


            //prima rispondo a tutti e poi invio msg
            while (msgrcv(my_msg_queue, &msg_queue, sizeof(msg_queue) - sizeof(long), getpid(), IPC_NOWAIT) ==
                   sizeof(msg_queue) - sizeof(long)) {


                switch (msg_queue.oggetto) {

                    case REPLY:


                        reserveSem(1);
                        f = fopen("log.txt", "a");
                        //  fprintf(f, "[%d] ricevuto reply %d\n", getpid(),msg_queue.student_mitt);
                        fclose(f);
                        /*
                         *  qualcuno ha risposto positivamente a un invito
                         * è il capo del gruppo a inserire i componenti al prorpio gruppo
                         */

                        if (SH_INDEX.libero) {

                            SH_INDEX.libero = FALSE;

                            G_INDEX.compagni[0] = getpid();
                            G_INDEX.compagni[1] = SH_MITT.matricola;

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


                            for (int i = 0; i < SH_INDEX.nof_elems; ++i) {
                                if (G_INDEX.compagni[i] == -1) {
                                    G_INDEX.compagni[i] = msg_queue.student_mitt;
                                    break;
                                }
                            }

                            if (G_INDEX.compagni[SH_INDEX.nof_elems - 1] > 0) {
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



                        for (int j = 0; j < nof_invites; ++j) {
                            if (SH_INDEX.utils[j].pid_invitato > 0 && SH_INDEX.utils[j].reply == FALSE) {
                                flag_wait = -1;
                                break;
                            }
                        }


                        if (flag_wait < 0) {

                            f = fopen("file.txt", "a");
                            fprintf(f, "[%d] invio wait a %d\n", getpid(), SH_MITT.matricola);
                            fclose(f);

                            /*
                             * se non ho ricevuto risposta da tutti invio un wait al mittente
                             * però tolgo il pid dalla lista
                             */

                            for (int i = 0; i < 4; ++i) {
                                if(SH_MITT.utils[i].pid_invitato == getpid()){
                                   SH_MITT.utils[i].pid_invitato=-1;
                                   SH_MITT.utils[i].reply=FALSE;
                                    break;
                                }
                            }

                            SH_MITT.nof_invites_send++;
                            //SET_REPLY_TRUE


                        } else if ((SH_INDEX.libero && (SH_INDEX.voto_AdE - SH_MITT.voto_AdE < 5 ||
                                                        SH_MITT.voto_AdE - SH_INDEX.voto_AdE > -5)) ||
                                   (SH_INDEX.nof_reject <= 0 && SH_INDEX.libero)) {


                            /*
                             * accetta se compatibile o se ha finito i reject
                             * viene impostato il mitta true
                             * viene inserito nel gruppo
                             * se è il primo
                             */

                            f = fopen("file.txt", "a");
                            fprintf(f, "[%d] accetto -> [%d]\n", getpid(), SH_MITT.matricola);
                            fclose(f);

                            SH_INDEX.libero = FALSE;


                            SET_REPLY_TRUE

                            msg_queue.mtype = msg_queue.student_mitt;
                            msg_queue.student_mitt = getpid();
                            msg_queue.oggetto = REPLY;

                            if (msgsnd(my_msg_queue, &msg_queue, sizeof(msg_queue) - sizeof(long), 0) < 0)
                                TEST_ERROR


                        } else if (SH_INDEX.nof_reject > 0) {

                            /*
                             * rifiuto per incompatibilità
                             */
                            f = fopen("file.txt", "a");
                            fprintf(f, "[%d] rifiuto -> [%d] reject %d\n", getpid(), SH_MITT.matricola,
                                    SH_INDEX.nof_reject);
                            fclose(f);

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
                (G_INDEX.capo == getpid() && G_INDEX.chiuso == FALSE && SH_INDEX.nof_invites_send > 0)) {

                //cerco studenti compatibili
                if (checkPariDispari(SH_TO_INVITE.matricola) && SH_INDEX.matricola != SH_TO_INVITE.matricola &&
                    (SH_TO_INVITE.nof_elems == SH_INDEX.nof_elems) && SH_TO_INVITE.libero) {


                    //controllo a quali studenti ho giù chiesto
                    for (int i = 0; i < 4; ++i) {
                        if (SH_INDEX.utils[i].pid_invitato == SH_TO_INVITE.matricola) {
                            //ho già chiesto quindi non invio
                            flag_no_spam = FALSE;
                        }
                    }

                    /*
                     * se non ho mai invitato aggiungo alla lista di invitati
                     */
                    if (flag_no_spam) {

                        f = fopen("file.txt", "a");
                       fprintf(f, "[%d] invio invito -> [%d]\n", getpid(), SH_TO_INVITE.matricola);
                        fclose(f);

                        for (int i = 0; i < 4; ++i) {
                            if (SH_INDEX.utils[i].pid_invitato == -1) {
                                SH_INDEX.utils[i].pid_invitato = SH_TO_INVITE.matricola;
                                SH_INDEX.utils[i].reply = FALSE;
                                break;
                            }
                        }

                        /*  SH_INDEX.utils[SH_INDEX.nof_invites_send - 1].pid_invitato = SH_TO_INVITE.matricola;
                          SH_INDEX.utils[SH_INDEX.nof_invites_send - 1].reply = FALSE;*/

                        // printf("[%d] nof -> %d mitt->%d\n", getpid(), SH_INDEX.nof_invites_send--,SH_TO_INVITE.matricola);
                        SH_INDEX.nof_invites_send--;


                        msg_queue.mtype = SH_TO_INVITE.matricola;
                        msg_queue.student_mitt = getpid();
                        msg_queue.oggetto = INVITO;


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


    //  msg_id = msgget(ID_KEY, IPC_CREAT | 0666);

    my_msg_queue = getMsgQueue();

    sem_id = semget(key, 2, IPC_CREAT | 0666);

    TEST_ERROR


    //inizializzo le variabili che caratterizzano uno studente
    reserveSem(1);

    //  printf("\n=== STUDENTE[%d] PRONTO ===\n", getpid());

    //printf("semid = %d key =  %d\n", sem_id, key);

    my_nof_elems = getNof_elems();
    my_voto_AdE = getVoto();

    nof_invites = shdata_pointer->config_values[3];


    SH_INDEX.matricola = getpid();
    SH_INDEX.nof_elems = my_nof_elems;
    SH_INDEX.voto_AdE = my_voto_AdE;
    SH_INDEX.libero = TRUE;

    SH_INDEX.nof_invites_send = nof_invites;
    SH_INDEX.nof_reject = shdata_pointer->config_values[4];

    for (int i = 0; i < nof_invites; ++i) {
        SH_INDEX.utils[i].pid_invitato = -1;
        SH_INDEX.utils[i].reply = FALSE;
    }

    for (int j = 0; j < POP_SIZE; ++j) {
        G_INDEX.capo = -1;
        G_INDEX.chiuso = FALSE;
        for (int i = 0; i < 4; ++i) {
            G_INDEX.compagni[i] = -1;
        }
    }


    /*  printf("my_nofelems= %d\n", my_nof_elems);
      printf("my_voto= %d\n", my_voto_AdE);
      printf("my_nof_send= %d\n", SH_INDEX.nof_invites_send);*/


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


        //se sono capo chiudo il mio gruppo
        if (G_INDEX.compagni[0] == getpid()) {
            // G_INDEX.chiuso = TRUE;
        }



        //stacco frammento di memoria da processo
        shmdt(shdata_pointer);

        msgctl(my_msg_queue, IPC_RMID, NULL);

        //  msgctl(msg_id, IPC_RMID, NULL);

        //  shmctl(shmem_id, IPC_RMID, NULL);


        exit(EXIT_SUCCESS);
    }
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




