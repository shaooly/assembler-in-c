CFLAGS = -Wall -ansi -pedantic

all: assembler

assembler: master.o preasm.o firstpassage.o secondpassage.o memory.o
	gcc $(CFLAGS) master.o preasm.o firstpassage.o secondpassage.o memory.o -o assembler

master.o: master.c
	gcc $(CFLAGS) -c master.c
		
preasm.o: preasm.c
	gcc $(CFLAGS) -c preasm.c
		
firstpassage.o: firstpassage.c
	gcc $(CFLAGS) -c firstpassage.c
		
secondpassage.o: secondpassage.c
	gcc $(CFLAGS) -c secondpassage.c
		
memory.o: memory.c
	gcc $(CFLAGS) -c memory.c

clean:
	rm -f master.o preasm.o firstpassage.o secondpassage.o memory.o
