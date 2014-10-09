CFLAGS = -g
CC = g++

OBJS = lisp.o edif.o inf.o main.o verilog.o net.o

netlist: $(OBJS)
	$(CC) $(CFLAGS) -o netlist $(OBJS)

clean:
	/bin/rm *.o *~
