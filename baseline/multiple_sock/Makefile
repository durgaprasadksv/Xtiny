CC = gcc
CFLAGS = -g -O2 -Wall -I .

# This flag includes the Pthreads library on a Linux box.
# Others systems will probably require something different.
LIB = -lpthread

all: tiny cgi

tiny: tiny.c csapp.o domainsock.o tpool.o threadpool.o
	$(CC) $(CFLAGS) -o tiny tiny.c domainsock.o tpool.o csapp.o threadpool.o $(LIB)

threadpool.o:
	$(CC) $(CFLAGS) -c threadpool.c

tpool.o: domainsock.o
	$(CC) $(CFLAGS) -c domainsock.o tpool.c 

csapp.o:
	$(CC) $(CFLAGS) -c csapp.c

domainsock.o:
	$(CC) $(CFLAGS) -c domainsock.c
cgi:
	(cd cgi-bin; make)

clean:
	rm -f *.o tiny *~
	(cd cgi-bin; make clean)