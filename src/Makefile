ifndef CC
CC = $(CC)
endif
CFLAGS=-Wall -O3
#CFLAGS=-Wall -g -O0
INCLUDES= -I.
LFLAGS= -L.
EXE = .exe

all: linedraw$(EXE) circledraw$(EXE) ellipsedraw$(EXE) wu$(EXE) pnmgen$(EXE) \
  rugca$(EXE)

linedraw.o: linedraw.c
	$(CC) ${CFLAGS} -c linedraw.c

linedraw$(EXE): linedraw.o
	$(CC) linedraw.o $(LFLAGS) -o linedraw$(EXE)

circledraw.o: circledraw.c
	$(CC) ${CFLAGS} -c circledraw.c

circledraw$(EXE): circledraw.o
	$(CC) circledraw.o $(LFLAGS) -o circledraw$(EXE)

ellipsedraw.o: ellipsedraw.c
	$(CC) ${CFLAGS} -c ellipsedraw.c

ellipsedraw$(EXE): ellipsedraw.o
	$(CC) ellipsedraw.o $(LFLAGS) -o ellipsedraw$(EXE)

wu.o: wu.c
	$(CC) ${CFLAGS} -c wu.c

wu$(EXE): wu.o
	$(CC) wu.o $(LFLAGS) -o wu$(EXE) -lm

pnmgen.o: pnmgen.c
	$(CC) ${CFLAGS} -c pnmgen.c

pnmgen$(EXE): pnmgen.o
	$(CC) pnmgen.o $(LFLAGS) -o pnmgen$(EXE)

rugca.o: rugca.c
	$(CC) ${CFLAGS} -DPRINT -c rugca.c

rugca$(EXE): rugca.o
	$(CC) rugca.o -o rugca$(EXE)

tidy:
	rm -f *.o core *~

clean:
	rm -f linedraw$(EXE) circledraw$(EXE) ellipsedraw$(EXE) wu$(EXE) pnmgen$(EXE) rugca$(EXE) *.o core *~
