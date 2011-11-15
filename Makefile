CFLAGS=-I./ -g -Wall
CC=gcc

HEADERS = sorb.h

sorb: sorb.o sorbT.o ${HEADERS}
	$(CC) $(CFLAGS) -o sorb sorb.o sorbT.o


clean:
	rm -f sorb *.o *~

sorb.tgz: sorb.o sorbT.o ${HEADERS} sorb
	rm -f sorb.tgz
	tar -czvf sorb.tgz sorb.c sorbT.c sorb.h sorb Makefile

.c.o: ${HEADERS}
	$(CC) $(CFLAGS) -c -o $*.o $<


