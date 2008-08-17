CFLAGS = -std=c89 -pedantic -Wall -O2 -I..
LDFLAGS = -s

all: runtests test_hash
.c.o:
	$(CC) -c $(CFLAGS) $<
runtests: runtests.o ../strutils.o
	$(CC) $(LDFLAGS) -o $@ $^
test_hash: test_hash.o ../hash.o
	$(CC) $(LDFLAGS) -o $@ $^
clean:
	rm -f *.o
