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


    printf(ANSI_COLOR_YELLOW"\n================ CARICAMENTO RISORSE COMPLETATO ===============\n"ANSI_COLOR_RESET
           ANSI_COLOR_BLUE "\n ======= ID IPC ======= \n" ANSI_COLOR_RESET
           " - sh_id  = %d\n"
           " - msg_pari = %d\n"
           " - msg_dispari = %d\n"
           " - sem_id = %d\n"
           " - sem_st = %d\n", sm_configValues_id, msg_pari, msg_dispari, sem_id, sem_st);


    printf(ANSI_COLOR_BLUE "\n ===== CONFIG VAR ===== \n" ANSI_COLOR_RESET
           " - POP_SIZE = %d\n"
           " - SIM_TIME = %d\n"
           " - nof_elem2: %d\n"
           " - nof_elem3: %d\n"
           " - nof_elem4: %d\n"
           " - nof_invites: %d\n"
           " - max_reject: %d\n",
           POP_SIZE, SIM_TIME, sm_configValues_pointer->config_values[0], sm_configValues_pointer->config_values[1],
           sm_configValues_pointer->config_values[2],
           sm_configValues_pointer->config_values[3], sm_configValues_pointer->config_values[4]);


    f = fopen("log.txt", "a");
    fprintf(f, "\n================ CARICAMENTO RISORSE COMPLETATO ===============\n"
               "\n ======= ID IPC ======= \n"
               " - sh_id  = %d\n"
               " - msg_pari = %d\n"
               " - msg_dispari = %d\n"
               " - sem_id = %d\n", sm_configValues_id, msg_pari, msg_dispari, sem_id);


    fprintf(f, "\n ===== CONFIG VAR ===== \n"
               " - POP_SIZE = %d\n"
               " - SIM_TIME = %d\n"
               " - nof_elem2: %d\n"
               " - nof_elem3: %d\n"
               " - nof_elem4: %d\n"
               " - nof_invites: %d\n"
               " - max_reject: %d\n",
            POP_SIZE, SIM_TIME, sm_configValues_pointer->config_values[0], sm_configValues_pointer->config_values[1],
            sm_configValues_pointer->config_values[2],
            sm_configValues_pointer->config_values[3], sm_configValues_pointer->config_values[4]);

    fclose(f);


    for (int i = 0; i < POP_SIZE; i++) {
        switch (fork()) {
            case -1:
                break;
            case 0:
                execve("student", arg_null, arg_null);
                break;
        }
    }

    printf(ANSI_COLOR_YELLOW"\n===================== CREAZIONE GRUPPI... =====================\n"ANSI_COLOR_RESET);

    //padre attende che tutti i figli terminino prima di terminare
    for (int j = 0; j < POP_SIZE; ++j) {
        int retStatus;
        waitpid(sm_students_pointer->students[j].matricola, &retStatus, 0);
    }

}


void signal_handler(int signalVal) {
    if (signalVal == SIGALRM || signalVal == SIGINT) {

        //blocco l'attività di tutti i processi figli
        releaseSem(sem_id, 0);

        for (int j = 0; j < POP_SIZE; ++j) {
            initSemInUse(sem_st, j);
        }


        printf(ANSI_COLOR_RED "\n================ PADRE[%d] SIM TIME TERMINATO ===============\n" ANSI_COLOR_RESET,
               getpid());


        printf(ANSI_COLOR_YELLOW "\n====== GUARDARE IL FILE DI LOG PER MAGGIORI INFORMAZIONI ======\n" ANSI_COLOR_RESET);

        reserveSem(sem_id, 1);
        f = fopen("log.txt", "a");
        fprintf(f, "\n============== ELENCO GRUPPI CREATI ===========\n\n");
        fclose(f);


        for (int i = 0; i < POP_SIZE; ++i) {
            int voto_max = 0;
            if (sm_students_pointer->groups[i].compagni[0] > 0 && sm_students_pointer->groups[i].chiuso) {

                int pidCapo = sm_students_pointer->groups[i].compagni[0];

                f = fopen("log.txt", "a");
                fprintf(f, "LEADER[%d] n_elems %d | n_invites_send %d\n",
                        sm_students_pointer->groups[i].compagni[0],
                        sm_students_pointer->students[pidCapo % POP_SIZE].nof_elems,
                        sm_students_pointer->students[pidCapo % POP_SIZE].nof_invites_send);
                fclose(f);

                /*
                 * assegno voto di SO
                 */
                for (int j = 0; j < 4; ++j) {
                    if (sm_students_pointer->groups[i].compagni[j] > 0) {
                        int x = sm_students_pointer->groups[i].compagni[j] % POP_SIZE;

                        f = fopen("log.txt", "a");
                        fprintf(f, "\t- Studente [%d] | voto_AdE: %d | nof_elems: %d\n",
                                sm_students_pointer->students[x].matricola,
                                sm_students_pointer->students[x].voto_AdE, sm_students_pointer->students[x].nof_elems);
                        fclose(f);

                        if (voto_max < sm_students_pointer->students[x].voto_AdE) {
                            voto_max = sm_students_pointer->students[x].voto_AdE;
                        }
                        sm_students_pointer->groups[i].voto = voto_max;
                    }
                }


                for (int z = 0; z < 4; ++z) {
                    if (sm_students_pointer->groups[i].compagni[z] > 0) {
                        int x = sm_students_pointer->groups[i].compagni[z] % POP_SIZE;
                        sm_students_pointer->students[x].voto_SO = sm_students_pointer->groups[i].voto;
                    }
                }
            }
        }

        printf("\n");

        f = fopen("log.txt", "a");
        fprintf(f, "\n============== ELENCO VOTI STUDENTI ===========\n\n");
        fclose(f);

        releaseSem(sem_id, 1);

        for (int j = 0; j < POP_SIZE; ++j) {

            f = fopen("log.txt", "a");
            if (sm_students_pointer->students[j].voto_SO > 0) {
                fprintf(f, "STUDENTE[%d] voto Sistemi Operativi: %d\n", sm_students_pointer->students[j].matricola,
                        sm_students_pointer->students[j].voto_SO);

            } else {
                fprintf(f, "STUDENTE[%d] voto Sistemi Operativi: BOCCIATO\n",
                        sm_students_pointer->students[j].matricola);
            }
            fclose(f);

            kill(sm_students_pointer->students[j].matricola, SIGINT);

            pid_t child = wait(&status);

            if (child == -1) {
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
                if (sm_students_pointer->students[i].voto_AdE == j) {
                    cont++;
                }
            }
            if (cont != 0)
                printf("Voto: %d | numero studenti: %d\n", j, cont);
        }

        for (int l = 0; l < POP_SIZE; ++l) {
            SUM_voto_AdE += sm_students_pointer->students[l].voto_AdE;
        }
        printf("\nMedia dei voti:%.2lf\n", (double) SUM_voto_AdE / POP_SIZE);



        //calcolo statistche per voto SO

        printf(ANSI_COLOR_BLUE"\n==================== STATISTICHE ESITI SO ==================\n" ANSI_COLOR_RESET);


        for (int j = 18; j <= 30; j++) {
            cont = 0;
            for (int i = 0; i < POP_SIZE; ++i) {
                if (sm_students_pointer->students[i].voto_SO == j) {
                    cont++;
                }
            }
            if (cont != 0)
                printf("Voto: %d | numero studenti: %d\n", j, cont);
        }

        int SUM_voto_SO = 0;
        int promossiSO = 0;
        for (int l = 0; l < POP_SIZE; ++l) {
            if (sm_students_pointer->students[l].voto_SO > 0) {
                SUM_voto_SO += sm_students_pointer->students[l].voto_SO;
                promossiSO++;
            }
        }

        printf("\nMedia voti:%.2lf\n", (double) SUM_voto_SO / promossiSO);


        if (shmctl(sm_students_id, IPC_RMID, NULL) == -1 ||
            shmctl(sm_configValues_id, IPC_RMID, NULL) == -1 || semctl(sem_id, 2, IPC_RMID) == -1 ||
            semctl(sem_st, POP_SIZE, IPC_RMID) == -1 || msgctl(msg_pari, IPC_RMID, NULL) == -1 ||
            msgctl(msg_dispari, IPC_RMID, NULL) == -1) {

            printf(ANSI_COLOR_YELLOW "\n==================== PULIZIA COMPLETATA ====================\n" ANSI_COLOR_RESET);
        } else {
            TEST_ERROR
        }

        f = fopen("log.txt", "a");
        fprintf(f, "\n============== FINE SIMULAZIONE ===========\n\n");
        fclose(f);

        printf(ANSI_COLOR_RED "\n==================== FINE SIMULAZIONE ======================\n" ANSI_COLOR_RESET);
    }
}

void init() {


    sem_id = semget(KEY, 2, IPC_CREAT | 0666);
    sem_st = semget(KEY_ST, POP_SIZE, IPC_CREAT | 0666);

    sm_configValues_id = shmget(KEY, sizeof(struct sm_configValues), IPC_CREAT | 0666);
    sm_configValues_pointer = (struct sm_configValues *) shmat(sm_configValues_id, NULL, 0);

    sm_students_id = shmget(KEY_ST, sizeof(struct sm_students), IPC_CREAT | 0666);
    sm_students_pointer = (struct sm_students *) shmat(sm_students_id, NULL, 0);

    msg_pari = msgget(KEY_PARI, IPC_CREAT | 0666);
    msg_dispari = msgget(KEY_DISPARI, IPC_CREAT | 0666);

    //mi accerto che le IPC siano state create
    if (sem_id == -1 || sm_configValues_id == -1 || msg_pari == -1 || msg_dispari == -1 || sem_st == -1) {
        TEST_ERROR
    }


    //inizializzo valore semafori
    semctl(sem_id, 0, SETVAL, POP_SIZE);
    initSemAvailable(sem_id, 1);

    for (int j = 0; j < POP_SIZE; ++j) {
        initSemAvailable(sem_st, j);
    }


    //leggo e salvo le variabili di configurazione su sm
    int *config_array = read_config();

    for (int i = 0; i < 5; ++i) {
        sm_configValues_pointer->config_values[i] = config_array[i];
    }


    //punto alla funzione che gestirà il segnale
    sa.sa_handler = &signal_handler;
    //installo handler e controllo errore
    sigaction(SIGALRM, &sa, &sa_old);
    sigaction(SIGINT, &sa, &sa_old);


    start_sim_time();
}

void start_sim_time() {
    int length = sizeof(sm_students_pointer->students) / sizeof(sm_students_pointer->students[0]);
    if (length == POP_SIZE) {
        alarm(SIM_TIME);
    }
}




