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


//errno impostato di default a 0 quando vi è un eccezione il SO assegna a quest avariabile il codice di errore
#define TEST_ERROR(x)  if(errno){\
 printf("[%d] Errore %d (%s) -> %s\n",getpid(),errno, strerror(errno),x);};




#define TRUE 1
#define FALSE 0

/*--- colors ---*/
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define PARI getpid()%2==0
#define DISPARI getpid()%2!=0

#define POP_SIZE 100
#define SIM_TIME 3

//=== Keys ===
#define KEY_DISPARI 1
#define KEY_PARI 2
#define KEY 3
#define KEY_ST 4


//=== macro per indici ===
#define GEST_INDEX(x) pid_students[x]%POP_SIZE
#define INDEX getpid()%POP_SIZE
#define INDEX_MITT msg_queue.student_mitt%POP_SIZE
#define SH_INDEX sm_students_pointer->students[INDEX]
#define SH_TO_INVITE sm_students_pointer->students[index_POPSIZE]
#define SH_MITT sm_students_pointer->students[INDEX_MITT]
#define G_INDEX sm_students_pointer->groups[INDEX]
#define G_MITT_INDEX sm_students_pointer->groups[INDEX_MITT]


//==== variabili processi ====
int status;
FILE *f;


//==== variabili strutture ====
int sem_id;
int sem_st;
int sm_configValues_id;
int sm_students_id;
int msg_pari;
int msg_dispari;

union semun {
    int val; /* for SETVAL */
    struct semid_ds *buf;  /* for IPC_STAT e IPC_SET */
    ushort *array; /* per GETALL e SETALL */
};


struct sigaction sa, sa_old;
struct my_msg msg_queue;
struct sm_configValues *sm_configValues_pointer;
struct sm_students *sm_students_pointer;

struct my_msg {
    long mtype;
    pid_t student_mitt;
};
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
struct sm_configValues {
    int config_values[5];
};
struct sm_students {
    struct student students[POP_SIZE];
    struct gruppo groups[POP_SIZE];
};


void init();

void signal_handler(int signalVal);

void start_sim_time();

int checkPariDispari(int matricola_to_compare);

void removeIPC();

int *read_config();

int getVoto();

int getNof_elems();

int getPercentNof_elems(int percent, int nStudents);

int getMsgQueue();

int getConfigValue(char line[1]);

//==== SEMAFORI ====//

//blocca risorsa x
int reserveSem(int sem_id, int semNum);

//rilascia risorsa x
int releaseSem(int sem_id, int semNum);

int getSemVal(int sem_id, int semNum);

int initSemInUse(int semId, int semNum);

int initSemAvailable(int semId, int semNum);

void exit_student();

void exit_gestore();

