CFLAGS  = -Wall -Wextra

all: pipe_lab fifo_writer fifo_reader

pipe_lab: pipe_lab.o
	cc pipe_lab.o -o pipe_lab

pipe_lab.o: pipe_lab.c
	cc $(CFLAGS) -c pipe_lab.c -o pipe_lab.o

fifo_writer: fifo_writer.o
	cc fifo_writer.o -o fifo_writer

fifo_writer.o: fifo_writer.c
	cc $(CFLAGS) -c fifo_writer.c -o fifo_writer.o

fifo_reader: fifo_reader.o
	cc fifo_reader.o -o fifo_reader

fifo_reader.o: fifo_reader.c
	cc $(CFLAGS) -c fifo_reader.c -o fifo_reader.o

clean:
	rm -f pipe_lab pipe_lab.o fifo_writer fifo_writer.o fifo_reader fifo_reader.o

.PHONY: clean