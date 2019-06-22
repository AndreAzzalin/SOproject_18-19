#include "libs_structs.h"

int npref[3];
int prob[100] = {0};
mid1 = 0;

int main(int argc, char *argv[]) {
    init();
    att(0);
    i = 0;
    op.sem_num = 0;
    op.sem_op = 0;
    semop(semid, &op, 1);


    while (1) {

        while (msgrcv(mid, &msg, sizeof(msg) - sizeof(long), pid, IPC_NOWAIT) == sizeof(msg) - sizeof(long)) {
            att(1);
            printf("\nSono(%d)\t", pid);


            if (sh_data->info[indx].accettato == 0) {
                if (nrejects == 0)
                    accept();
                else if (voto <= sh_data->info[msg.mpid % POP_SIZE].vote) {
                    accept();
                } else {
                    refuse();
                }
            } else if (nrejects > 0 && sh_data->info[msg.mpid % POP_SIZE].pref == pref)
                accept();
            else if (ninvits == 0)
                refuse();
            else
                refuse();

            sig(1);
            break;
        }


        msg.mpid = pid;

        if (ninvits > 0 && (sh_data->info[indx].accettato == 0 ||
                            (sh_data->gruppi[indx].compagni[0] == pid && sh_data->gruppi[indx].chiuso == 0))) {
            att(1);
            if (pid % 2 == sh_data->info[i].matr % 2 && sh_data->info[i].accettato == 0 &&
                sh_data->gruppi[i].compagni[0] == 0 && pid != sh_data->info[i].matr && sh_data->info[i].pref <= pref) {
                msg.mtype = sh_data->info[i].matr;
                ninvits--;
                if (msgsnd(mid, &msg, sizeof(msg) - sizeof(long), 0) < 0) TEST_ERROR;
            }
            sig(1);
            i++;

            if (i == POP_SIZE)
                i = 0;
        }
    }
    exit(EXIT_SUCCESS);
}

void init() {
    read_config();

    srand(pid = getpid());

    openipc();

    //assegno preferenza gruppo (così non va bene)
    pref = prob[rand() % 100];

    //assegno voto
    voto = rand() % 13 + 18;
    indx = pid % POP_SIZE;

    //acquisisci risorsa sh_data
    att(1);
    sh_data->info[indx].matr = pid;
    sh_data->info[indx].vote = voto;
    sh_data->info[indx].pref = pref;
    printf("\tStudente #%d\tmatricola %d\tvoto %d\tpreferenza %d\n", indx, pid, voto, pref);
    sh_data->info[indx].accettato = 0;
    //rilascia risorsa sh_data
    sig(1);
    atexit(aexit);
}

void openipc() {
    sa.sa_handler = &signal_handler;
    sigaction(SIGINT, &sa, &saold);
    //message per padre
    msgid = msgget(getppid(), IPC_CREAT);
    TEST_ERROR;
    //se il pid(matricola) è pari
    if (pid % 2) {
        mid = msgget(K1, 0666);
    //se il pid(matricola) è dispari
    } else {
        mid = msgget(K0, 0666);
    }

    TEST_ERROR;
    semid = semget(getppid(), 0, IPC_CREAT);
    TEST_ERROR;
    shrmem = shmget(getppid(), 0, IPC_CREAT);
    TEST_ERROR;
    sh_data = (struct shared_data *) shmat(shrmem, NULL, 0);
    TEST_ERROR;
}


//operazioni su semaforo
//wait -1 su sem x
void att(x) {
    op.sem_num = x;
    op.sem_op = -1;
    semop(semid, &op, 1);

}

//signal 1
void sig(x) {
    op.sem_num = x;
    op.sem_op = 1;
    semop(semid, &op, 1);
}

//gestione gruppo
void refuse() {
    nrejects--;
    printf("RIFIUTO(%d)\n", msg.mpid);
}

void accept() {
    pidcapo = msg.mpid % POP_SIZE;
    if (sh_data->gruppi[pidcapo].chiuso == 0) {
        ninvits = 0;
        sh_data->gruppi[indx].chiuso = 1;
        sh_data->info[indx].accettato = 1;
        sh_data->info[pidcapo].accettato = 1;

        if (sh_data->gruppi[pidcapo].cont == 0) {
            sh_data->gruppi[pidcapo].compagni[sh_data->gruppi[pidcapo].cont++] = pid;
            sh_data->gruppi[pidcapo].voto = sh_data->info[pidcapo].vote;
            sh_data->gruppi[pidcapo].compagni[sh_data->gruppi[pidcapo].cont++] = msg.mpid;
        } else if (sh_data->gruppi[pidcapo].cont <= sh_data->info[pidcapo].pref) {
            sh_data->gruppi[pidcapo].compagni[sh_data->gruppi[pidcapo].cont++] = msg.mpid;
            if (sh_data->gruppi[pidcapo].voto < voto)
                sh_data->gruppi[pidcapo].voto = voto;
        }
        if (sh_data->gruppi[pidcapo].cont == sh_data->info[pidcapo].pref) {
            sh_data->gruppi[pidcapo].chiuso = 1;
            ninvits = 0;
            if (msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0) < 0) TEST_ERROR;
        }
        printf("ACCETTO(%d)\n", msg.mpid);
    } else refuse();
}

//leggi da file
void read_config() {
    FILE *f = fopen("file.config", "r");
    int i, j = 0;
    char s[10];


    while (fscanf(f, "%s", s) != -1) {
        if (i % 2 == 1) {
            if (i < 6) {
                npref[j++] = atoi(s);
            }
        }
        if (i == 7) ninvits = atoi(s);
        if (i == 9) nrejects = atoi(s);
        ++i;
    }

    pref = 2;
    cont = npref[j = 0];

    for (i = 0; i < 100; ++i) {
        if (i == cont) {
            pref++;
            cont += npref[++j];
        }
        prob[i] = pref;
    }
    TEST_ERROR;
    fclose(f);
}

//fine processo
void aexit() {
    op.sem_num = 0;
    op.sem_op = 0;
    semop(semid, &op, 1);

    for (i = 0; i < POP_SIZE; i++) {
        j = 0;
        if (sh_data->gruppi[i].compagni[0] != 0) {
            while (j < sh_data->gruppi[i].cont) {
                if (sh_data->gruppi[i].compagni[j] == pid) {
                    if (sh_data->gruppi[i].voto > 17) {
                        printf("Matricola %d il mio esito finale_%d\n", pid, sh_data->gruppi[i].voto);
                    } else printf("Matricola %d il mio esito finale_BOCCIATO\n", pid);
                }
                j++;
            }
        }
    }
    printf("\n");
    shmdt(sh_data);
}


void signal_handler(signalvalue) {
    switch (signalvalue) {
        case SIGINT: {
            if (sh_data->gruppi[indx].chiuso == 0 && sh_data->gruppi[indx].compagni[0] == pid)
                sh_data->gruppi[indx].chiuso = 1;
            att(0);
            exit(EXIT_SUCCESS);
        }
    }
}


