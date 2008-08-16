CC = cc
YACC = bison
LEX = flex

CFLAGS = -std=c89 -pedantic -Wall -O2
LDFLAGS = -s
LFLAGS =
YFLAGS = -d

OBJS = cyacc.o clex.o parse.o ironout.o

all: ironout runtests

parse.o: parse.h ast.h utils.h
cyacc.c: cyacc.y
	$(YACC) $(YFLAGS) -o $@ $<
clex.c: clex.l
	$(LEX) $(LFLAGS) -o $@ $<
.c.o:
	$(CC) -c $(CFLAGS) $<
ironout: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

runtests: runtests.o strutils.o
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm -f cyacc.[ch] clex.c *.output *.o
ctags:
	etags *.[hc]
