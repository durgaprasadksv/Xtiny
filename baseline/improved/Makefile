CC = gcc
CFLAGS = -g -O2 -Wall -I .

# This flag includes the Pthreads library on a Linux box.
# Others systems will probably require something different.
LIB = -lpthread

all: tiny cgi

tiny: tiny.c csapp.o thpool.o
	$(CC) $(CFLAGS) -o tiny tiny.c thpool.o csapp.o $(LIB)

csapp.o:
	$(CC) $(CFLAGS) -c csapp.c
thpool.o:
	$(CC) $(CFLAGS) -c thpool.c
cgi:
	(cd cgi-bin; make)

clean:
	rm -f *.o tiny *~
	(cd cgi-bin; make clean)