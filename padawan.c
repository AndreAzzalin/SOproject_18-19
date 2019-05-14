#include "lib.h"


int main(int argc, char *argv[]) {

    init();
    TEST_ERROR


    while (1) {

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


    //inizializzo le variabili che caratterizzano uno studente
    reserveSem(1);

    printf("\n=== STUDENTE[%d] ===\n", getpid());

    printf("semid = %d key =  %d\n", sem_id, key);


    shdata_pointer->students[getSemVal(0) - 1].matricola = getpid();
    shdata_pointer->students[getSemVal(0) - 1].voto_AdE = getVoto();
    shdata_pointer->students[getSemVal(0) - 1].nof_elems = getNof_elems();


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

        exit(EXIT_SUCCESS);
    }
}



