#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>

#define TEST_ERROR if(errno){\
        dprintf(STDERR_FILENO,"%s:%d: PID=%5d: Error %d (%s)\n", \
                           __FILE__, __LINE__, getpid(), errno, strerror(errno));}


#define TRUE 1
#define FALSE 0

/*--- colors ---*/
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define PARI getpid()%2==0
#define DISPARI getpid()%2!=0

#define POP_SIZE 1000
#define SIM_TIME 5

#define ID_KEY 'a'
#define KEY_PARI 2
#define KEY_DISPARI 1

#define INDEX getpid()%POP_SIZE
#define INDEX_MITT msg_queue.student_mitt%POP_SIZE
#define SH_INDEX shdata_pointer->students[INDEX]
#define SH_TO_INVITE shdata_pointer->students[index_POPSIZE]
#define SH_MITT shdata_pointer->students[INDEX_MITT]
#define G_INDEX shdata_pointer->groups[INDEX]
#define G_MITT_INDEX shdata_pointer->groups[INDEX_MITT]


//==== variabili processi ====
int status;
char *arg_null[] = {NULL};
FILE *f;


//==== variabili strutture ====
key_t key;
int sem_id;
int shmem_id;
int msg_pari;
int msg_dispari;

struct sigaction sa, sa_old;
struct my_msg msg_queue;
struct shdata *shdata_pointer;


struct my_msg {
    long mtype;
    pid_t student_mitt;
};


union semun {
    int val; /* for SETVAL */
    struct semid_ds *buf;  /* for IPC_STAT e IPC_SET */
    ushort *array; /* per GETALL e SETALL */
};



//==== FUNZIONI UTIL ====//

key_t setKey() {
    key_t newKey;
    if ((newKey = ftok(".", ID_KEY)) == (key_t) -1) {
        TEST_ERROR
    }
    return newKey;
}

int getVoto() {
    return rand() % 13 + 18;
}

int getPercentNof_elems(int percent, int nStudents) {
    float val = (float) (nStudents * percent) / 100;
    return (int) round(val);
}

int getConfigValue(char line[1]) {
    int value = 0;

    /* get the first token */
    char *token = strtok(line, ":");

    /* walk through other tokens */
    while (token != NULL) {
        value = atoi(token);
        token = strtok(NULL, line);
    }


    return value;
}

int *read_config() {

    FILE *f = fopen("opt.conf", "r+");

    char nof_elems2_toConvert[20];
    char nof_elems3_toConvert[20];
    char nof_elems4_toConvert[20];
    char nof_invites_toConvert[20];
    char max_reject_toConvert[20];

    fscanf(f, "%s %s %s %s %s", nof_elems2_toConvert, nof_elems3_toConvert, nof_elems4_toConvert, nof_invites_toConvert,
           max_reject_toConvert);


    int nof_elems2 = getPercentNof_elems(getConfigValue(nof_elems2_toConvert), POP_SIZE);
    int nof_elems3 = getPercentNof_elems(getConfigValue(nof_elems3_toConvert), POP_SIZE);
    int nof_elems4 = getPercentNof_elems(getConfigValue(nof_elems4_toConvert), POP_SIZE);

    int nof_invites = getConfigValue(nof_invites_toConvert);
    int max_reject = getConfigValue(max_reject_toConvert);

    TEST_ERROR
    fclose(f);


    static int configVariables[5];

    configVariables[0] = nof_elems2;
    configVariables[1] = nof_elems3;
    configVariables[2] = nof_elems4;
    configVariables[3] = nof_invites;
    configVariables[4] = max_reject;


    return configVariables;
}



//==== OPERAZIONI SEMAFORI ====//

//blocca risorsa x
int reserveSem(int semNum) {
    struct sembuf sops;
    sops.sem_num = semNum;
    sops.sem_op = -1;
    sops.sem_flg = 0;

    return semop(sem_id, &sops, 1);
}

//rilascia risorsa x
int releaseSem(int semNum) {
    struct sembuf sops;
    sops.sem_num = semNum;
    sops.sem_op = 1;
    sops.sem_flg = 0;
    return semop(sem_id, &sops, 1);
}

int getSemVal(int semNum) {
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


//==== STRUTTURE DATI ====//

struct student {
    pid_t matricola;
    int nof_elems;
    int voto_AdE;
    int libero;
    int nof_invites_send;
    int nof_reject;
    int voto_SO;
    pid_t pid_invitato[4];
};

struct gruppo {
    pid_t compagni[4];
    int chiuso;
    int voto;
};

struct shdata {
    struct student students[POP_SIZE];
    struct gruppo groups[POP_SIZE];
    int config_values[5];
};




//==== COMMON ====//

void init();

int getNof_elems() {

//questo metodo è eseguito solo con lock del semaforo 1
    if (shdata_pointer->config_values[0] > 0) {
        shdata_pointer->config_values[0]--;
        return 2;

    } else if (shdata_pointer->config_values[1] > 0) {
        shdata_pointer->config_values[1]--;
        return 3;

    } else if (shdata_pointer->config_values[2] > 0) {
        shdata_pointer->config_values[2]--;
        return 4;
    }

}

void signal_handler(int signalVal);

void start_sim_time();

int checkPariDispari(int matricola_to_compare);

int getMsgQueue();




