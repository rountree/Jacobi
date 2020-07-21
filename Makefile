all: Makefile main parameters
	gcc -O2 -std=c99 -Wall -o jacobi main.o parameters.o

parameters: Makefile parameters.c parameters.h
	gcc -O2 -std=c99 -Wall -c parameters.c

main: main.c
	gcc -O2 -std=c99 -Wall -c main.c

clean:
	rm -f jacobi *.o

