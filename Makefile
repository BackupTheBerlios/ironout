CC = cc
YACC = bison
LEX = flex

CFLAGS = -std=c99 -pedantic -Wall -O2
LDFLAGS = -s
LFLAGS =
YFLAGS = -dv

OBJS = gram.tab.o scan.o parse.o

all: parse

gram.tab.c: gram.y
	$(YACC) $(YFLAGS) $<

scan.c: scan.l
	$(LEX) $(LFLAGS) -t $< >$@

parse.h: gram.tab.h
parse.o: parse.h
scan.o: parse.h
.c.o:
	$(CC) -c $(CFLAGS) $<
parse: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm -f *.tab.[ch] scan.c *.output *.o
ctags:
	etags *.[hc]
