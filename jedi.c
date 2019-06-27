#include "lib.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_RESET   "\x1b[0m"


int main(int argc, char *argv[]) {

    printf(ANSI_COLOR_GREEN "\n"
           "/***\n"
           " *    ______                     _   _          _____  _____   \n"
           " *    | ___ \\                   | | | |        /  ___||  _  |  \n"
           " *    | |_/ / __ ___   __ _  ___| |_| |_ ___   \\ `--. | | | |  \n"
           " *    |  __/ '__/ _ \\ / _` |/ _ \\ __| __/ _ \\   `--. \\| | | |  \n"
           " *    | |  | | | (_) | (_| |  __/ |_| || (_) | /\\__/ /\\ \\_/ /  \n"
           " *    \\_|  |_|  \\___/ \\__, |\\___|\\__|\\__\\___/  \\____/  \\___/   \n"
           " *                     __/ |                                   \n"
           " *                    |___/                                    \n"
           " *     _____  _____  __   _____     _______  _____  __   _____ \n"
           " *    / __  \\|  _  |/  | |  _  |   / / __  \\|  _  |/  | |  _  |\n"
           " *    `' / /'| |/' |`| |  \\ V /   / /`' / /'| |/' |`| | | |_| |\n"
           " *      / /  |  /| | | |  / _ \\  / /   / /  |  /| | | | \\____ |\n"
           " *    ./ /___\\ |_/ /_| |_| |_| |/ /  ./ /___\\ |_/ /_| |_.___/ /\n"
           " *    \\_____/ \\___/ \\___/\\_____/_/   \\_____/ \\___/ \\___/\\____/ \n"
           " *                                                             \n"
           " *                                                             \n"
           " */\n" ANSI_COLOR_RESET);

    printf(ANSI_COLOR_RED "=============== PADRE[%d] STARTING SIMULATION ===============\n" ANSI_COLOR_RESET,
           getpid());


    init();
    TEST_ERROR


    printf(ANSI_COLOR_GREEN"\n================== INITIALIZATION  COMPLETE ====================\n"ANSI_COLOR_RESET
           ANSI_COLOR_YELLOW "\n ======= ID IPC ======= \n" ANSI_COLOR_RESET
           " - sh_id  = %d\n"
           " - msg_pari = %d\n"
           " - msg_dispari = %d\n"
           " - sem_id = %d\n", shmem_id, msg_pari, msg_dispari, sem_id);

    reserveSem(1);
    printf(ANSI_COLOR_YELLOW "\n ===== CONFIG VAR ===== \n" ANSI_COLOR_RESET
           " - Nof_Students = %d\n"
           " - nof_elem2: %d\n"
           " - nof_elem3: %d\n"
           " - nof_elem4: %d\n"
           " - nof_invites: %d\n"
           " - max_reject: %d\n",
           POP_SIZE, shdata_pointer->config_values[0], shdata_pointer->config_values[1],
           shdata_pointer->config_values[2],
           shdata_pointer->config_values[3], shdata_pointer->config_values[4]);
    releaseSem(1);


    for (int i = 0; i < POP_SIZE; i++) {
        switch (fork()) {
            case -1:
                TEST_ERROR;
                break;
            case 0:
                execve("padawan", arg_null, arg_null);
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


        printf(ANSI_COLOR_RED "\n==================== PADRE[%d] SIGALRM =====================\n" ANSI_COLOR_RESET,
               getpid());
        for (int j = 0; j < POP_SIZE; ++j) {

            //invio segnale di terminazione ai figli
            kill(shdata_pointer->students[j].matricola, SIGINT);

            //aspetto che il figlio gestisca il segnale
            pid_t child = wait(&status);


            if (child > 0 && WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                f = fopen("file.txt", "a");
                fprintf(f, "[%d] studente = %d | voto = %d | nof_elems = %d | lib %d | nof_invites %d\n", j,
                        shdata_pointer->students[j].matricola,
                        shdata_pointer->students[j].voto_AdE,
                        shdata_pointer->students[j].nof_elems, shdata_pointer->students[j].libero,
                        shdata_pointer->students[j].nof_invites_send);

                for (int k = 0; k < 4; ++k) {
                    fprintf(f, "- %d | %d \n", shdata_pointer->students[j].utils[k].pid_invitato,
                            shdata_pointer->students[j].utils[k].reply);
                }

                fclose(f);

            }


        }


        printf("\n==== GRUPPI =====\n");
        for (int i = 0; i < POP_SIZE; ++i) {

            int voto_max = 0;
            if (shdata_pointer->groups[i].compagni[0] > 0 && shdata_pointer->groups[i].chiuso) {

                int pidCapo = shdata_pointer->groups[i].compagni[0];

                f = fopen("file.txt", "a");
                printf("gruppo[%d] n_elems %d n_invites %d | closed: %d | test %d \n",
                       shdata_pointer->groups[i].compagni[0],
                       shdata_pointer->students[pidCapo % POP_SIZE].nof_elems,
                       shdata_pointer->students[pidCapo % POP_SIZE].nof_invites_send, shdata_pointer->groups[i].chiuso,
                       shdata_pointer->students[pidCapo % POP_SIZE].matricola);

                fprintf(f, "gruppo[%d] n_elems %d n_invites_send %d | closed: %d | test %d \n",
                        shdata_pointer->groups[i].compagni[0],
                        shdata_pointer->students[pidCapo % POP_SIZE].nof_elems,
                        shdata_pointer->students[pidCapo % POP_SIZE].nof_invites_send, shdata_pointer->groups[i].chiuso,
                        shdata_pointer->students[pidCapo % POP_SIZE].matricola);


                for (int j = 0; j < 4; ++j) {

                    if (shdata_pointer->groups[i].compagni[j] > 0) {

                        int x = shdata_pointer->groups[i].compagni[j] % POP_SIZE;

                        fprintf(f, "- %d | %d\n", shdata_pointer->students[x].matricola,
                                shdata_pointer->students[x].voto_AdE);


                        for (int k = 0; k < shdata_pointer->students[pidCapo % POP_SIZE].nof_elems; ++k) {
                            if (voto_max < shdata_pointer->students[x].voto_AdE) {
                                voto_max = shdata_pointer->students[x].voto_AdE;
                            }

                            shdata_pointer->students[x].voto_SO = voto_max;

                        }
                    }
                }


                fclose(f);


                for (int j = 0; j < 4; ++j) {
                    if (shdata_pointer->groups[i].compagni[j] > 0 && shdata_pointer->groups[i].chiuso) {
                        int x = shdata_pointer->groups[i].compagni[j] % POP_SIZE;
                        printf("- %d | %d\n", shdata_pointer->students[x].matricola,
                               shdata_pointer->students[x].voto_AdE);
                    }
                }
                printf("\n");
            }
        }


        for (int l = 0; l < POP_SIZE; ++l) {
            f = fopen("file.txt", "a");
            fprintf(f, "[%d]  voto_ade = %d | lib %d | voto_so %d | noof_inv %d | reject %d\n",
                    shdata_pointer->students[l].matricola,
                    shdata_pointer->students[l].voto_AdE,
                    shdata_pointer->students[l].libero, shdata_pointer->students[l].voto_SO,
                    shdata_pointer->students[l].nof_invites_send, shdata_pointer->students[l].nof_reject);
            fclose(f);
        }

        releaseSem(1);



        if (!semctl(sem_id, 2, IPC_RMID) && !shmctl(shmem_id, IPC_RMID, NULL)) {
            printf(ANSI_COLOR_YELLOW "\n==================== PULIZIA COMPLETATA ====================\n" ANSI_COLOR_RESET);
        } else {
            TEST_ERROR
        }

        printf(ANSI_COLOR_RED "\n====================== END SIMULATION ======================\n" ANSI_COLOR_RESET);

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




