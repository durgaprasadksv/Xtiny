CC = gcc
CFLAGS = -g -O2 -Wall -I .

# This flag includes the Pthreads library on a Linux box.
# Others systems will probably require something different.
LIB = -lpthread

all: xtiny cgi snapy

xtiny: tiny.c csapp.o threadpool.o
	$(CC) $(CFLAGS) -o Xtiny tiny.c threadpool.o csapp.o $(LIB)

snapy: snapy.c csapp.o threadpool.o content.o
	$(CC) $(CFLAGS) -o snapy snapy.c threadpool.o csapp.o content.o $(LIB)

content.o:
	$(CC) $(CFLAGS) -c content.c

csapp.o:
	$(CC) $(CFLAGS) -c csapp.c
threadpool.o:
	$(CC) $(CFLAGS) -c threadpool.c
cgi:
	(cd cgi-bin; make)

clean:
	rm -f *.o tiny *~
	(cd cgi-bin; make clean)
