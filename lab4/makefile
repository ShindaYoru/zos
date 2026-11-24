CFLAGS  = -Wall -Wextra

all: sender receiver


sender: sender.o
	cc sender.o -o sender

sender.o: sender.c
	cc $(CFLAGS) -c sender.c -o sender.o

receiver: receiver.o
	cc receiver.o -o receiver

receiver.o: receiver.c
	cc $(CFLAGS) -c receiver.c -o receiver.o

clean:
	rm -f sender sender.o receiver receiver.o

.PHONY: clean