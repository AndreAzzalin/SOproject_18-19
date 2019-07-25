#include "lib.h"

//variabilii util solo per student
int my_nof_elems;
int my_voto_AdE;
int nof_invites;

int my_msg_queue;
int flag_no_spam = TRUE;
int index_POPSIZE = 0;



int main(int argc, char *argv[]) {

    init();

    while (TRUE) {


        //uso il sem0 per capire quando tutti i processi sono stati caricati
        if (!getSemVal(sem_id, 0)) {

            //se nella coda di messaggi ci sono msg con mtype = al mio pid, leggo il messaggio
            while (msgrcv(my_msg_queue, &msg_queue, sizeof(msg_queue) - sizeof(long), getpid(), IPC_NOWAIT) ==
                   sizeof(msg_queue) - sizeof(long)) {

                reserveSem(sem_st, INDEX);
                reserveSem(sem_st, INDEX_MITT);

                if (!SH_INDEX.libero) {

                    /*
                     * se per qualche motivo non sono più libero toglimi dalla lista
                     */

                    for (int i = 0; i < 4; ++i) {
                        if (SH_MITT.pid_invitato[i] == getpid()) {
                            SH_MITT.pid_invitato[i] = -1;
                            break;
                        }
                    }


                    if (SH_MITT.nof_invites_send != nof_invites) {
                        SH_MITT.nof_invites_send++;
                    }


                } else if ((SH_INDEX.libero && (SH_INDEX.voto_AdE - SH_MITT.voto_AdE < 5 ||
                                                SH_MITT.voto_AdE - SH_INDEX.voto_AdE > -5)) ||

                           (SH_INDEX.nof_reject <= 0 && SH_INDEX.libero)) {
                    /*
                     * accetta se compatibile o se ha finito i reject
                     * viene impostato il mitta true
                     * viene inserito nel gruppo
                     * se è il primo
                     */


                    /*
                     * faccio creare il gruppo se sono il primo ad accettare
                     * se il gruppo già esiste ed è aperto mi inserisco
                     */
                    if (SH_MITT.libero) {


                        SH_MITT.libero = FALSE;
                        SH_INDEX.libero = FALSE;


                        G_MITT_INDEX.compagni[0] = SH_MITT.matricola;
                        G_MITT_INDEX.compagni[1] = getpid();


                        if (SH_MITT.nof_elems == 2) {
                            G_MITT_INDEX.chiuso = TRUE;
                        }


                    } else if ((G_MITT_INDEX.compagni[0] == SH_MITT.matricola) && (G_MITT_INDEX.chiuso == FALSE)) {

                        /*
                         * sono capo e devo chiudere il gruppo
                         * rimane caso gruppi formati da 3 o 4 studenti
                         */

                        SH_MITT.libero = FALSE;
                        SH_INDEX.libero = FALSE;


                        for (int i = 0; i < SH_MITT.nof_elems; ++i) {
                            if (G_MITT_INDEX.compagni[i] == -1) {
                                G_MITT_INDEX.compagni[i] = getpid();
                                break;
                            }
                        }

                        if (G_MITT_INDEX.compagni[SH_MITT.nof_elems - 1] > 0) {
                            G_MITT_INDEX.chiuso = TRUE;
                        }
                    }
                } else if (SH_INDEX.nof_reject > 0) {

                    /*
                     * rifiuto per incompatibilità
                     */



                    SH_INDEX.nof_reject--;
                }


                releaseSem(sem_st, INDEX);
                releaseSem(sem_st, INDEX_MITT);

            }

            /*
             *
             *  SEZIONE INVIO
             *
             */



            if (index_POPSIZE != INDEX && getSemVal(sem_st, INDEX) > 0 && getSemVal(sem_st, index_POPSIZE) > 0) {

                reserveSem(sem_st, INDEX);


                flag_no_spam = TRUE;

                /*
                 * se ho inviti disponibili
                 * se sono libero
                 * se sono capo e non ho ancora chiuso il gruppo
                 */
                if ((SH_INDEX.nof_invites_send > 0 && SH_INDEX.libero) ||
                    (G_INDEX.compagni[0] == getpid() && G_INDEX.chiuso == FALSE && SH_INDEX.nof_invites_send > 0)) {

                    reserveSem(sem_st, index_POPSIZE);

                    //cerco studenti compatibili
                    if (checkPariDispari(SH_TO_INVITE.matricola) && SH_TO_INVITE.libero &&
                        (SH_INDEX.matricola != SH_TO_INVITE.matricola) &&
                        (SH_TO_INVITE.nof_elems == SH_INDEX.nof_elems)) {


                        //controllo a quali studenti ho giù chiesto
                        for (int i = 0; i < 4; ++i) {
                            if (SH_INDEX.pid_invitato[i] == SH_TO_INVITE.matricola) {
                                //ho già chiesto quindi non invio
                                flag_no_spam = FALSE;
                            }
                        }

                        /*
                         * se non ho mai invitato aggiungo alla lista di invitati
                         */
                        if (flag_no_spam) {


                            for (int i = 0; i < 4; ++i) {
                                if (SH_INDEX.pid_invitato[i] == -1) {
                                    SH_INDEX.pid_invitato[i] = SH_TO_INVITE.matricola;
                                    break;
                                }
                            }

                            SH_INDEX.nof_invites_send--;
                            msg_queue.mtype = SH_TO_INVITE.matricola;
                            msg_queue.student_mitt = getpid();

                            if (msgsnd(my_msg_queue, &msg_queue, sizeof(msg_queue) - sizeof(long), 0) < 0)
                                TEST_ERROR("Invio invito")

                        }
                    }

                    releaseSem(sem_st, index_POPSIZE);

                } else if (SH_INDEX.libero && SH_INDEX.nof_invites_send <= 0 && SH_INDEX.nof_reject <= 0) {
                    /*
                     * se ho finito gli inviti, non sono ancora in nessnu gruppo
                     * e ho un voto compreso tra 27 e 30 mi creo un gruppo da solo
                     */

                    if (30 - SH_INDEX.voto_AdE <= 3) {
                        G_INDEX.compagni[0] = getpid();
                        G_INDEX.chiuso = TRUE;
                        SH_INDEX.libero = FALSE;
                    }
                }

                releaseSem(sem_st, INDEX);

                index_POPSIZE++;
            } else {
                index_POPSIZE++;
            }

            if (index_POPSIZE - 1 == POP_SIZE)
                index_POPSIZE = 0;
        }
    }
}

void init() {

    //per random
    srand(getpid());

    //mi allaccio alle strutture IPC

    sem_id = semget(KEY, 2, IPC_CREAT | 0666);
    sem_st = semget(KEY_ST, POP_SIZE, IPC_CREAT | 0666);

    sm_configValues_id = shmget(KEY, sizeof(struct sm_configValues), IPC_CREAT | 0666);
    sm_configValues_pointer = (struct sm_configValues *) shmat(sm_configValues_id, NULL, 0);

    sm_students_id = shmget(KEY_ST, sizeof(struct sm_students), IPC_CREAT | 0666);
    sm_students_pointer = (struct sm_students *) shmat(sm_students_id, NULL, 0);

    my_msg_queue = getMsgQueue();


    //inizializzo le variabili che caratterizzano uno studente

    reserveSem(sem_st, INDEX);

    reserveSem(sem_id, 1);
    my_nof_elems = getNof_elems();
    my_voto_AdE = getVoto();
    nof_invites = sm_configValues_pointer->config_values[3];
    int nof_reject = sm_configValues_pointer->config_values[4];
    releaseSem(sem_id, 1);


    SH_INDEX.matricola = getpid();
    SH_INDEX.nof_elems = my_nof_elems;
    SH_INDEX.voto_AdE = my_voto_AdE;
    SH_INDEX.voto_SO = 0;
    SH_INDEX.libero = TRUE;

    SH_INDEX.nof_invites_send = nof_invites;
    SH_INDEX.nof_reject = nof_reject;

    for (int i = 0; i < nof_invites; ++i) {
        SH_INDEX.pid_invitato[i] = -1;
    }


    G_INDEX.chiuso = FALSE;
    for (int i = 0; i < 4; ++i) {
        G_INDEX.compagni[i] = -1;
    }


    releaseSem(sem_st, INDEX);


    //decremento sem0
    reserveSem(sem_id, 0);

    //punto alla funzione che gestirà il segnale
    sa.sa_handler = &signal_handler;
    //installo handler e controllo errore
    sigaction(SIGINT, &sa, &sa_old);

    TEST_ERROR("Init student")
}

void signal_handler(int signalVal) {
    if (signalVal == SIGINT) {

        if (SH_INDEX.voto_SO > 0) {
            printf("STUDENTE[%d] voto Sistemi Operativi: %d\n", SH_INDEX.matricola, SH_INDEX.voto_SO);
        } else {
            printf("STUDENTE[%d] voto Sistemi Operativi: BOCCIATO \n", SH_INDEX.matricola);
        }

        //stacco frammento di memoria da processo
        shmdt(sm_configValues_pointer);
        shmdt(sm_students_pointer);

        exit(0);
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

int getNof_elems() {

//questo metodo è eseguito solo con lock del semaforo 1
    if (sm_configValues_pointer->config_values[0] > 0) {
        sm_configValues_pointer->config_values[0]--;
        return 2;

    } else if (sm_configValues_pointer->config_values[1] > 0) {
        sm_configValues_pointer->config_values[1]--;
        return 3;

    } else if (sm_configValues_pointer->config_values[2] > 0) {
        sm_configValues_pointer->config_values[2]--;
        return 4;
    }

}

int getVoto() {
    return rand() % 13 + 18;
}








