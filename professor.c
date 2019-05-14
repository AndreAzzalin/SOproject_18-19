#include "libs_structs.h"



int main(int argc, char const *argv[]){
  pid = getpid();
  init();

    //student sono i figli di professor
  for(i=0; i < POP_SIZE; i++){
    if(fork()){
    }else{
       execve("student", 0, 0);
      TEST_ERROR;
    }
  }


  op.sem_num = 0;
  op.sem_op = 0;
  semop(semid, &op, 1);
  semctl(semid, 0, SETVAL, POP_SIZE);

  alarm(7);

  while(msgrcv(msgid, &msg, sizeof(msg)-sizeof(long), 0, 0) == sizeof(msg)-sizeof(long)){
    printf("\n\nRICEVUTO MSG: dice che %d ha chiuso il suo gruppo\n\n", (int)msg.mtype);
  }
}


void init(){

  //creo code di messaggi
  mid0 = msgget(K0, IPC_CREAT|0666);
  mid1 = msgget(K1, IPC_CREAT|0666);
  msgid = msgget(pid, IPC_CREAT|0666);

  //creo set di 2 semafori
  semid = semget(pid, 2, IPC_CREAT|0666);

  //creo memoria condivisa
  shrmem = shmget(pid, sizeof(struct shared_data), IPC_CREAT|0666);
  sh_data = (struct shared_data*)shmat(shrmem, NULL, 0);

  //inizializzo  memoria condivisa
  for(i=0; i<POP_SIZE; i++){
    sh_data->gruppi[i].voto = 0;
    sh_data->gruppi[i].chiuso = 0;
    sh_data->gruppi[i].cont = 0;
    for(j=0; j<4; j++)
     sh_data->gruppi[i].compagni[j] = 0;
  }

  sh_data->ngruppi=0;

  //imposto il valore al set di semafori POP_SIZE e 1
  semctl(semid, 0, SETVAL, POP_SIZE);
  semctl(semid, 1, SETVAL, 1);

  sa.sa_handler = &signal_handler;
  sigaction(SIGALRM, &sa, &saold);
  TEST_ERROR;


  printf("\n\t-_-_-INIZIALIONE-_-_-\n\n");
  atexit(aexit);
}


void aexit(){
  semctl(semid, 2, IPC_RMID);
  msgctl(msgid, IPC_RMID, NULL);

  for(i=0; i<POP_SIZE; i++){
    wait(&status);
  }

  printf("\n\t_-_-_-MEDIA_STUDENTI-_-_-_\n\n");
  int cont, v = 0;

  for(j=18; j<=30; j++){
    cont = 0;
    for(i=0; i<POP_SIZE; ++i){
      if(sh_data->info[i].vote == j){
        cont++;
      }
    }
    printf("\t#studenti:%d con voto:%d\n", cont, j);
  }
  v = 0;

  for(i=0; i<POP_SIZE; ++i){
    v+=sh_data->info[i].vote;
  }

  printf("\tvoto medio Architettura %d\n\n", v/POP_SIZE);
  int conts=0;
  cont = 0;
  v = 0;

  for(j=18; j<=30; j++){
    cont = 0;
    for(i=0; i<POP_SIZE; ++i){
      if(sh_data->gruppi[i].voto == j){
        cont += sh_data->gruppi[i].cont;
      }
    }
    printf("\t#studenti:%d con voto:%d\n", cont, j);
  }
  cont = 0;
  v=0;

  for(i=0; i<POP_SIZE; ++i){
    if(sh_data->gruppi[i].cont>0 && sh_data->gruppi[i].voto >0){
      v += sh_data->gruppi[i].voto;
      cont++;
    }
  }
  if(cont)
   printf("\tvoto medio Sistemi Op %d\n", v/cont);
  else
    printf("\tvoto medio Sistemi Op %d\n", 0);


  printf("\n\t-_-_-_END_SIMULATION_-_-_-\n\n");
  shmctl(shrmem, IPC_RMID , NULL);
}


void signal_handler(int signalvalue){
  switch(signalvalue){
    case SIGALRM:{
      sleep(1);
      printf("\n");
      printf("\n\t_-_-_-GRUPPI-_-_-_\n");
      for(i=0; i<POP_SIZE; i++){
        kill(sh_data->info[i].matr, SIGINT);
        if(sh_data->info[i].accettato == 0){
          sh_data->gruppi[i].compagni[0] = sh_data->info[i].matr;
          sh_data->gruppi[i].voto = 0;
          sh_data->gruppi[i].chiuso = 0;
          sh_data->gruppi[i].cont = 1;
        }
        j=0;
        if(sh_data->gruppi[i].compagni[0] != 0){
          while(j<sh_data->gruppi[i].cont){
            if(j == 0) printf("\n\tcapo: %d", sh_data->gruppi[i].compagni[j]);
            if(j > 0) printf(" compagno[%d]: %d ", j, sh_data->gruppi[i].compagni[j]);
            if(sh_data->gruppi[i].chiuso == 1 && sh_data->info[sh_data->gruppi[i].compagni[j]%POP_SIZE].pref != sh_data->gruppi[i].cont){
              sh_data->gruppi[i].voto -= 3;
            }
            j++;
          }
        }
      }
      printf("\n");
      msgctl(mid0, IPC_RMID, NULL);
      msgctl(mid1, IPC_RMID, NULL);
      printf("\n");
      exit(EXIT_SUCCESS);
    }
  }
}


void att(x){
  op.sem_num = x;
  op.sem_op = -1;
  semop(semid, &op, 1);
}
void sig(x){
  op.sem_num = x;
  op.sem_op = 1;
  semop(semid, &op, 1);
}
