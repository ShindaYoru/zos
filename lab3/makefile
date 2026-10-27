CFLAGS  = -Wall -Wextra

all: pipe_lab

pipe_lab: pipe_lab.o
	cc pipe_lab.o -o pipe_lab

pipe_lab.o: pipe_lab.c
	cc $(CFLAGS) -c pipe_lab.c -o pipe_lab.o

clean:
	rm -f pipe_lab pipe_lab.o

.PHONY: clean