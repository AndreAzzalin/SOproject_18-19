#include "lib.h"

int reserveSem(int sem_id, int semNum) {
    struct sembuf sops;
    sops.sem_num = semNum;
    sops.sem_op = -1;
    sops.sem_flg = 0;

    return semop(sem_id, &sops, 1);
}

//rilascia risorsa x
int releaseSem(int sem_id, int semNum) {
    struct sembuf sops;
    sops.sem_num = semNum;
    sops.sem_op = 1;
    sops.sem_flg = 0;
    return semop(sem_id, &sops, 1);
}

int getSemVal(int sem_id, int semNum) {
    union semun arg;
    return semctl(sem_id, semNum, GETVAL, arg);
}

int initSemInUse(int semId, int semNum) {
    union semun arg;
    arg.val = 0;
    return semctl(semId, semNum, SETVAL, arg);
}

int initSemAvailable(int semId, int semNum) {
    union semun arg;
    arg.val = 1;
    return semctl(semId, semNum, SETVAL, arg);
}