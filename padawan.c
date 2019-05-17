#include "lib.h"


int main(int argc, char *argv[]) {

    init();
    TEST_ERROR


    while (1) {

        //uso il sem0 per capire quando tutti i processi sono stati caricati

        if (getSemVal(0) == 0) {

            sleep(1);
            printf("---[%d]---"
                   "semval= %d\n", getpid(), getSemVal(0));

             while (msgrcv(msg_id, &msg_queue, sizeof(msg_queue) - sizeof(long), getpid(), IPC_NOWAIT) == sizeof(msg_queue) - sizeof(long)) {
                 printf("sono[%d] RICEVUTO msg da (%d)\n", getpid(),msg_queue.student_dest);
             }


            //destinatario
            msg_queue.mtype = getpid()+1;
             //mittente
            msg_queue.student_dest = getpid();

             if (msgsnd(msg_id, &msg_queue, sizeof(msg_queue) - sizeof(long), 0) < 0) {
                  TEST_ERROR
              } else {
                  printf("[%d] msg send with %d\n", getpid(), msg_queue.student_dest);
              }



        }






        /*  printf("---[%d]---\n",getpid());
          msg_queue.student_dest = getpid();

          if (msgsnd(msg_id, &msg_queue, sizeof(msg_queue) - sizeof(long), 0)) {
              printf("[%d] msg send\n", getpid());
          }
          releaseSem(1);


          reserveSem(1);
          while (msgrcv(msg_id, &msg_queue, sizeof(msg_queue) - sizeof(long), getpid(), 0)) {
              printf("[%d] msg received\n", getpid());
          }
          releaseSem(1);*/


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


    msg_id = msgget(KO, IPC_CREAT | 0666);


    TEST_ERROR


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



