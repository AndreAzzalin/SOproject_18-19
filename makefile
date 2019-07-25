ST_SOURCE= student student.o lib_sem.o
GE_SOURCE= gestore gestore.o lib_sem.o

all: gestore student clean run

student: student.o lib_sem.o
	gcc -o $(ST_SOURCE)
#collego -lm perchè la libreria math.h è implementata da libm.so e libc.so , libm non è linkata di default al compilatore mentre libc si
gestore: gestore.o lib_sem.o
	gcc -o $(GE_SOURCE) -lm

student.o: student.c lib.h
	gcc -c student.c

gestore.o: gestore.c lib.h
	gcc -c gestore.c

lib_sem.o: lib_sem.c lib.h
	gcc -c lib_sem.c

clean:
	rm -f gestore.o lib_sem.o student.o

run: gestore
	./gestore

