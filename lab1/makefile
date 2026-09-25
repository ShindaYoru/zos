CFLAGS  = -Wall -Wextra

all: mycat mygrep mychmod myls

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

mychmod.o: mychmod.c
	cc $(CFLAGS) -c mychmod.c -o mychmod.o

myls: myls.o
	cc myls.o -o myls

myls.o: myls.c
	cc $(CFLAGS) -c myls.c -o myls.o



clean:
	rm -f mycat mycat.o mygrep mygrep.o mychmod mychmod.o myls myls.o




.PHONY: clean