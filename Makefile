CC=gcc
COPTS= -Wall -D_XOPEN_SOURCE

all: submit.cgi

clean:
	-rm -rf *.o *~ core

clobber:
	-rm -rf *.o *~ core submit.cgi

submit.cgi: submit.o
	$(CC) $(COPTS) submit.o -o submit.cgi
	cp submit.cgi ../cgi-bin
	chmod a+s ../cgi-bin/submit.cgi

.c.o:
	$(CC) -c $(COPTS) $<

submit.o: submit.c
