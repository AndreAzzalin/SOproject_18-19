#include "lib.h"


int main(int argc, char *argv[]) {

    printf("==== PADRE[%d] STARTING SIMULATION ====\n", getpid());
    init();
    TEST_ERROR


    printf("==== END INITIALIZATION ====\n"
           " \n - Nof_Students = %d\n"
           " - sh_id  = %d\n"
           " - msg_id  = %d\n"
           " - sem_id = %d\n", POP_SIZE, shmem_id, msg_id, sem_id);

    reserveSem(1);
    printf(" - nof_elem2: %d\n"
           " - nof_elem3: %d\n "
           " - nof_elem4: %d\n  "
           " - nof_invites: %d\n"
           " - max_reject: %d\n",
           shdata_pointer->config_values[0], shdata_pointer->config_values[1], shdata_pointer->config_values[2],
           shdata_pointer->config_values[3], shdata_pointer->config_values[4]);
    releaseSem(1);


    for (int i = 0; i < POP_SIZE; i++) {
        switch (fork()) {
            case -1:
                TEST_ERROR;
                break;
            case 0:
                execve("padawan", NULL, NULL);
                break;
        }
    }


    //padre attende che tutti i figli terminino prima di terminare
    for (int i = 0; i < POP_SIZE; ++i) {
        wait(&status);
    }


    exit(EXIT_SUCCESS);
}


void signal_handler(int signalVal) {
    if (signalVal == SIGALRM || signalVal == SIGINT) {

        reserveSem(1);

        /*
         * assegno voti
         */



        printf("\n==== PADRE[%d] SIGALRM =====\n", getpid());
        for (int j = 0; j < POP_SIZE; ++j) {

            //invio segnale di terminazione ai figli
            kill(shdata_pointer->students[j].matricola, SIGINT);

            //aspetto che il figlio gestisca il segnale
            pid_t child = wait(&status);


            if (child > 0 && WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                f = fopen("file.txt", "a");
                 fprintf(f,"[%d] studente = %d | voto = %d | nof_elems = %d | lib %d | nof_invites %d\n", j,
                            shdata_pointer->students[j].matricola,
                            shdata_pointer->students[j].voto_AdE,
                            shdata_pointer->students[j].nof_elems, shdata_pointer->students[j].libero,  shdata_pointer->students[j].nof_invites_send);

                for (int k = 0; k < 4; ++k) {
                    fprintf(f ,"- %d \n", shdata_pointer->students[j].utils[k].pid_invitato);
                }

                fclose(f);

            }


        }


        printf("\n==== GRUPPI =====\n");
        for (int i = 0; i < POP_SIZE; ++i) {

            if (shdata_pointer->students[i].libero) {
                // printf("[%d] sono ancora free\n",shdata_pointer->students[i].matricola);
            }


            //shdata_pointer->groups[i].compagni[0] > 0
            if (shdata_pointer->groups[i].compagni[0] > 0) {

                int pidCapo = shdata_pointer->groups[i].compagni[0];

                f = fopen("file.txt", "a");
                printf("gruppo[%d] n_elems %d n_invites %d | closed: %d | test %d \n",
                       shdata_pointer->groups[i].compagni[0],
                       shdata_pointer->students[pidCapo % POP_SIZE].nof_elems,
                       shdata_pointer->students[pidCapo % POP_SIZE].nof_invites_send, shdata_pointer->groups[i].chiuso,
                       shdata_pointer->students[pidCapo % POP_SIZE].matricola);

                fprintf( f, "gruppo[%d] n_elems %d n_invites_send %d | closed: %d | test %d \n",
                       shdata_pointer->groups[i].compagni[0],
                       shdata_pointer->students[pidCapo % POP_SIZE].nof_elems,
                       shdata_pointer->students[pidCapo % POP_SIZE].nof_invites_send, shdata_pointer->groups[i].chiuso,
                       shdata_pointer->students[pidCapo % POP_SIZE].matricola);



                fclose(f);




                printf("------ compari ----\n");

                for (int j = 0; j < 4; ++j) {
                    if (shdata_pointer->groups[i].compagni[j] > 0) {
                        printf("- %d\n", shdata_pointer->groups[i].compagni[j]);
                    }
                }
                printf("\n");
            }
        }


        releaseSem(1);


        printf("\n==== PULIZIA COMPLETATA ====\n"
               "semctl = %d\n"
               "shmctrl = %d\n"
               "smgctrl = %d %d", semctl(sem_id, 2, IPC_RMID), shmctl(shmem_id, IPC_RMID, NULL),
               msgctl(msg_pari, IPC_RMID, NULL), msgctl(msg_dispari, IPC_RMID, NULL));


        printf("\n==== END SIMULATION ====\n");

        exit(EXIT_SUCCESS);
    }
}

void init() {




    key = setKey();

    //creo set di 2 semafori
    // uno per sh_data e uno per
    sem_id = semget(key, 2, IPC_CREAT | 0666);
    shmem_id = shmget(key, sizeof(struct shdata), IPC_CREAT | 0666);
    shdata_pointer = (struct shdata *) shmat(shmem_id, NULL, 0);

    msg_pari = msgget(KEY_PARI, IPC_CREAT | 0666);
    msg_dispari = msgget(KEY_DISPARI, IPC_CREAT | 0666);

    //mi accerto che le IPC siano state create
    if (sem_id == -1 || shmem_id == -1 || msg_id == -1) {
        TEST_ERROR
    }


    semctl(sem_id, 0, SETVAL, POP_SIZE);
    semctl(sem_id, 1, SETVAL, 1);


    //leggo e salvo le variabili di configurazione su sm
    reserveSem(1);
    int *config_array = read_config();

    for (int i = 0; i < 5; ++i) {
        shdata_pointer->config_values[i] = config_array[i];
    }
    releaseSem(1);


    //punto alla funzione che gestirà il segnale
    sa.sa_handler = &signal_handler;
    //installo handler e controllo errore
    sigaction(SIGALRM, &sa, &sa_old);
    TEST_ERROR

    start_sim_time();
}

void start_sim_time() {
    reserveSem(1);
    int length = sizeof(shdata_pointer->students) / sizeof(shdata_pointer->students[0]);
    if (length == POP_SIZE) {
        alarm(SIM_TIME);
    }

    releaseSem(1);
}


