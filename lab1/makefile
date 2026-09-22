CFLAGS  = -Wall -Wextra

all: mycat mygrep

mycat: mycat.o
	cc mycat.o -o mycat

mycat.o: mycat.c
	cc $(CFLAGS) -c mycat.c -o mycat.o

mygrep: mygrep.o
	cc mygrep.o -o mygrep

mygrep.o: mygrep.c
	cc $(CFLAGS) -c mygrep.c -o mygrep.o

mychmod: mychmod.o
	cc mychmod.o -o mychmod

mygrep.o: mychmod.c
	cc $(CFLAGS) -c mychmod.c -o mychmod.o
clean:
	rm -f mycat mycat.o mygrep mygrep.o mychmod mychmod.o

.PHONY: clean