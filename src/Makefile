ifndef CC
CC = $(CC)
endif
CFLAGS=-Wall -O2
INCLUDES= -I.
LFLAGS= -L.
CC = gcc
EXE = .exe

all: linedraw$(EXE) circledraw$(EXE) wu$(EXE) pnmgen$(EXE)

linedraw$(EXE): linedraw.o
	$(CC) linedraw.o $(LFLAGS) -o linedraw$(EXE)

circledraw$(EXE): circledraw.o
	$(CC) circledraw.o $(LFLAGS) -o circledraw$(EXE)

wu$(EXE): wu.o
	$(CC) wu.o $(LFLAGS) -o wu$(EXE)

pnmgen$(EXE): pnmgen.o
	$(CC) pnmgen.o $(LFLAGS) -o pnmgen$(EXE)

tidy:
	rm -f *.o core *~

clean:
	rm -f linedraw$(EXE) circledraw$(EXE) wu$(EXE) pnmgen$(EXE) *.o core *~