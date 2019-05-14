#include "lib.h"

struct student {
    pid_t matricola;
    int nof_elems;
    int voto_AdE;
};

struct shdata{
    struct student students[POP_SIZE];
};

struct shdata* shdata_pointer;
