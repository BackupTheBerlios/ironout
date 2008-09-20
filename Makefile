CC = cc
YACC = bison
LEX = flex

CFLAGS = -std=c89 -pedantic -Wall -O2 -g
LDFLAGS =
LFLAGS =
YFLAGS = -d

OBJS = cyacc.o clex.o parse.o ironout.o ast.o strutils.o hash.o utils.o \
	block.o name.o cfile.o find.o project.o path.o src.o rename.o \
	typedefs.o

all: ironout

parse.o: parse.h ast.h utils.h
ast.o: ast.h
strutils.o: strutils.h
hash.o: hash.h
cyacc.c: c.y
	$(YACC) $(YFLAGS) -o $@ $<
clex.c: c.l
	$(LEX) $(LFLAGS) -o $@ $<
.c.o:
	$(CC) -c $(CFLAGS) $<
ironout: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

test: all
	$(MAKE) -C tests/ all
clean:
	rm -f cyacc.[ch] clex.c *.o
	$(MAKE) -C tests/ clean
ctags:
	etags *.[hc]
