CFLAGS = -std=c89 -pedantic -Wall -O2
LDFLAGS = -s

all: runtests
.c.o:
	$(CC) -c $(CFLAGS) -I.. $<
runtests: runtests.o ../strutils.o
	$(CC) $(LDFLAGS) -o $@ $^
clean:
	rm -f *.o
