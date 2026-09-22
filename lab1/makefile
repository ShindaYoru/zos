CFLAGS  = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L

all: mycat mygrep

mycat: mycat.o
	cc mycat.o -o mycat

mycat.o: mycat.c
	cc $(CFLAGS) -c mycat.c -o mycat.o

mygrep: mygrep.o
	cc mygrep.o -o mygrep

mygrep.o: mygrep.c
	cc $(CFLAGS) -c mygrep.c -o mygrep.o

clean:
	rm -f mycat mycat.o mygrep mygrep.o

.PHONY: clean