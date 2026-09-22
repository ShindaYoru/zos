CFLAGS  = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L

mycat: mycat.o
	cc mycat.o -o mycat

mycat.o: mycat.c
	cc $(CFLAGS) -c mycat.c -o mycat.o

clean:
	rm -f mycat mycat.o

.PHONY: clean