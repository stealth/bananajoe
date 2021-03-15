CC=cc
CFLAGS=-std=c11 -Wall -O2 -pedantic -fPIC -pie

all: 7350bananajoe

shell: 7350bananajoe run

run:
	uname -a
	./7350bananajoe

7350bananajoe: 7350bananajoe.c
	$(CC) $(CFLAGS) 7350bananajoe.c -o 7350bananajoe

