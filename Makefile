CC = cc
YACC = bison
LEX = flex

CFLAGS = -std=c99 -pedantic -Wall -O2
LDFLAGS = -s
LFLAGS =
YFLAGS = -dv

OBJS = gram.tab.o scan.o parse.o ironout.o

all: ironout

parse.h: gram.tab.h
parse.o: parse.h
scan.o: parse.h
gram.tab.c: gram.y
	$(YACC) $(YFLAGS) $<
scan.c: scan.l
	$(LEX) $(LFLAGS) -t $< >$@
.c.o:
	$(CC) -c $(CFLAGS) $<
ironout: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm -f *.tab.[ch] scan.c *.output *.o
ctags:
	etags *.[hc]
