CC=gcc

all: bxe

bxe: bxe.o encrypt.o variableint.o include/sodium.h
	[ -d out ] || mkdir out
	$(CC) bxe.o encrypt.o variableint.o -lsodium -o out/bxe

bxe.o: encrypt.h include/sodium.h bxe.c
	$(CC) -I include -c bxe.c -lsodium

encrypt.o: encrypt.c variableint.h variableint.o
	$(CC) -c encrypt.c

variableint.o: variableint.c
	$(CC) -c variableint.c

clean: 
	rm bxe.o encrypt.o variableint.o out/bxe
	rm -r out