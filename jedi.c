#include "lib.h"


int cont, SUM_voto_AdE = 0;


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
           " ***/\n" ANSI_COLOR_RESET);

    printf(ANSI_COLOR_RED "================= PADRE[%d] AVVIA SIMULAZIONE ===============\n" ANSI_COLOR_RESET,
           getpid());


    init();
    TEST_ERROR


    printf(ANSI_COLOR_YELLOW"\n================ CARICAMENTO RISORSE COMPLETATO ===============\n"ANSI_COLOR_RESET
           ANSI_COLOR_BLUE "\n ======= ID IPC ======= \n" ANSI_COLOR_RESET
           " - sh_id  = %d\n"
           " - msg_pari = %d\n"
           " - msg_dispari = %d\n"
           " - sem_id = %d\n", shmem_id, msg_pari, msg_dispari, sem_id);

    reserveSem(1);
    printf(ANSI_COLOR_BLUE "\n ===== CONFIG VAR ===== \n" ANSI_COLOR_RESET
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

    printf(ANSI_COLOR_YELLOW"\n===================== CREAZIONE GRUPPI... =====================\n"ANSI_COLOR_RESET);

    //padre attende che tutti i figli terminino prima di terminare
    for (int i = 0; i < POP_SIZE; ++i) {
        wait(&status);
    }


    exit(EXIT_SUCCESS);
}


void signal_handler(int signalVal) {
    if (signalVal == SIGALRM || signalVal == SIGINT) {

        reserveSem(1);

        //blocco l'attività di tutti i processi figli
        releaseSem(0);


        printf(ANSI_COLOR_RED "\n================ PADRE[%d] SIM TIME TERMINATO ===============\n" ANSI_COLOR_RESET,
               getpid());


        printf(ANSI_COLOR_YELLOW "\n====== GUARDARE IL FILE DI LOG PER MAGGIORI INFORMAZIONI ======\n" ANSI_COLOR_RESET);


        for (int i = 0; i < POP_SIZE; ++i) {


            int voto_max = 0;
            if (shdata_pointer->groups[i].compagni[0] > 0 && shdata_pointer->groups[i].chiuso) {

                int pidCapo = shdata_pointer->groups[i].compagni[0];

                f = fopen("file.txt", "a");
                fprintf(f, "gruppo[%d] n_elems %d n_invites_send %d | closed: %d | test %d \n",
                        shdata_pointer->groups[i].compagni[0],
                        shdata_pointer->students[pidCapo % POP_SIZE].nof_elems,
                        shdata_pointer->students[pidCapo % POP_SIZE].nof_invites_send, shdata_pointer->groups[i].chiuso,
                        shdata_pointer->students[pidCapo % POP_SIZE].matricola);


                /*
                 * assegno voto di SO
                 */
                for (int j = 0; j < 4; ++j) {
                    if (shdata_pointer->groups[i].compagni[j] > 0) {
                        int x = shdata_pointer->groups[i].compagni[j] % POP_SIZE;

                        fprintf(f, "- %d | %d\n", shdata_pointer->students[x].matricola,
                                shdata_pointer->students[x].voto_AdE);

                        if (voto_max < shdata_pointer->students[x].voto_AdE) {
                            voto_max = shdata_pointer->students[x].voto_AdE;
                        }
                        shdata_pointer->groups[i].voto = voto_max;
                    }
                }


                for (int z = 0; z < 4; ++z) {
                    if (shdata_pointer->groups[i].compagni[z] > 0) {
                        int x = shdata_pointer->groups[i].compagni[z] % POP_SIZE;
                        shdata_pointer->students[x].voto_SO = shdata_pointer->groups[i].voto;
                    }
                }

                fclose(f);
            }
        }

        printf("\n");
        for (int j = 0; j < POP_SIZE; ++j) {

            kill(shdata_pointer->students[j].matricola, SIGINT);

            //aspetto che il figlio gestisca il segnale
            pid_t child = wait(&status);

            if (child > 0 || WIFEXITED(status) || WEXITSTATUS(status)) {
                TEST_ERROR
            }
        }


        printf(ANSI_COLOR_RED"\n==================== DEBRIEF SIMULAZIONE ====================\n" ANSI_COLOR_RESET);

        /*
         * calcolo statistche per voto architettura
         */
        printf(ANSI_COLOR_BLUE"\n==================== STATISTICHE ESITI AdE ==================\n" ANSI_COLOR_RESET);

        for (int j = 18; j <= 30; j++) {
            cont = 0;
            for (int i = 0; i < POP_SIZE; ++i) {
                if (shdata_pointer->students[i].voto_AdE == j) {
                    cont++;
                }
            }
            if (cont != 0)
                printf("Voto: %d | numero studenti: %d\n", j, cont);
        }

        for (int l = 0; l < POP_SIZE; ++l) {
            SUM_voto_AdE += shdata_pointer->students[l].voto_AdE;
        }
        printf("\nMedia dei voti:%.2lf\n", (double) SUM_voto_AdE / POP_SIZE);


        /*
       * calcolo statistche per voto SO
       */
        printf(ANSI_COLOR_BLUE"\n==================== STATISTICHE ESITI SO ==================\n" ANSI_COLOR_RESET);


        for (int j = 18; j <= 30; j++) {
            cont = 0;
            for (int i = 0; i < POP_SIZE; ++i) {
                if (shdata_pointer->students[i].voto_SO == j) {
                    cont++;
                }
            }
            if (cont != 0)
                printf("Voto: %d | numero studenti: %d\n", j, cont);
        }

        int SUM_voto_SO = 0;
        int promossiSO = 0;
        for (int l = 0; l < POP_SIZE; ++l) {
            if (shdata_pointer->students[l].voto_SO > 0) {
                SUM_voto_SO += shdata_pointer->students[l].voto_SO;
                promossiSO++;
            }
        }

        printf("\nMedia voti:%.2lf\n", (double) SUM_voto_SO / promossiSO);

        releaseSem(1);


        if (!semctl(sem_id, 2, IPC_RMID) && !shmctl(shmem_id, IPC_RMID, NULL)) {
            printf(ANSI_COLOR_YELLOW "\n==================== PULIZIA COMPLETATA ====================\n" ANSI_COLOR_RESET);
        } else {
            TEST_ERROR
        }


        printf(ANSI_COLOR_RED "\n==================== FINE SIMULAZIONE ======================\n" ANSI_COLOR_RESET);

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
    if (sem_id == -1 || shmem_id == -1 || msg_pari == -1 || msg_dispari == -1) {
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




