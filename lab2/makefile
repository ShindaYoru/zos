CFLAGS  = -Wall -Wextra

all: fork_demo

fork_demo: fork_demo.o
	cc fork_demo.o -o fork_demo

fork_demo.o: fork_demo.c
	cc $(CFLAGS) -c fork_demo.c -o fork_demo.o

clean:
	rm -f fork_demo fork_demo.o

.PHONY: clean