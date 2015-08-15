ALEPH = ~/aleph-w

CLANGPATH = /home/lrleon/LLVM-3.6.1/bin

CXX = $(CLANGPATH)/clang++
CC = $(CLANGPATH)/clang

FFLAGS=

FLEX=flex ${FFLAGS}

YACC=bison

WARN = -Wall -Wextra -Wcast-align -Wno-sign-compare -Wno-write-strings \
       -Wno-parentheses -Wno-invalid-source-encoding -Wno-deprecated-register

FLAGS = -g -O0 -std=c++14 $(WARN)

OPT=-O3 -DWITHOUT_NANA -DNDEBUG -std=c++14 $(WARN)

INCLUDE = -I. -I $(ALEPH) 

LIBS = -L $(ALEPH) \
       -lAleph -lnana -lm -lgsl -lgslcblas -lgmp -lmpfr -lasprintf -lpthread -lc

OBJ = grafo.o net.o

all: test test-1 test-2 test-3 test-io test-1-op test-2-op test-3-op transform-data transform-data-op transform-data-2 transform-data-2-op test-load-grafo test-load-productores test-load-productos load-net repl test-net test-lex

clean:
	rm -f test testcsv test-1 test-2 test-3 test-1-op test-2-op test-3-op test-io transform-data transform-data-op transform-data-2 transform-data-2-op test-load-grafo test-load-productores test-load-productos load-net repl net-lex.C test-net test-lex $(OBJ) net-lex.o text.tab.o

csvparser.o: csvparser.h csvparser.c
	$(CC) $(INCLUDE) $*.c -g -O0 -c

grafo.o: grafo.H tablas.H grafo.C
	$(CXX) $(FLAGS) $(INCLUDE) -c grafo.C

net.o: grafo.H tablas.H grafo.C net.H
	$(CXX) $(FLAGS) $(INCLUDE) -c net.C

test-1: test-1.C csvparser.o tablas.H grafo.H
	$(CXX) $(FLAGS) $(INCLUDE) $@.C -o $@ $(LIBS)

test-1-op: test-1.C tablas.H grafo.H
	$(CXX) $(OPT) $(INCLUDE) test-1.C -o $@ $(LIBS)

test-2: test-2.C tablas.H grafo.H
	$(CXX) $(FLAGS) $(INCLUDE) $@.C -o $@ $(LIBS)

test-2-op: test-2.C tablas.H grafo.H
	$(CXX) $(OPT) $(INCLUDE) test-2.C -o $@ $(LIBS)

test-3: test-3.C tablas.H grafo.H
	$(CXX) $(FLAGS) $(INCLUDE) $@.C -o $@ $(LIBS)

test-3-op: test-3.C tablas.H grafo.H
	$(CXX) $(OPT) $(INCLUDE) test-3.C -o $@ $(LIBS)

testcsv: testcsv.C csvparser.o
	$(CXX) $(FLAGS) $(INCLUDE) $@.C -o $@ csvparser.o $(LIBS)

test-csv: test-csv.C parse-csv.H
	$(CXX) $(FLAGS) $(INCLUDE) $@.C -o $@ $(LIBS)

test-io: test-io.C tablas.H grafo.H
	$(CXX) $(FLAGS) $(INCLUDE) $@.C -o $@ $(LIBS)

transform-data: transform-data.C tablas.H grafo.H
	$(CXX) $(FLAGS) $(INCLUDE) $@.C -o $@ $(LIBS)

transform-data-op: transform-data.C tablas.H grafo.H
	$(CXX) $(OPT) $(INCLUDE) transform-data.C -o $@ $(LIBS)

transform-data-2: transform-data-2.C tablas.H grafo.H net.H
	$(CXX) $(FLAGS) $(INCLUDE) $@.C -o $@ $(LIBS)

transform-data-2-op: transform-data-2.C tablas.H grafo.H net.H
	$(CXX) $(OPT) $(INCLUDE) transform-data-2.C -o $@ $(LIBS)

test-load-grafo: test-load-grafo.C tablas.H grafo.H net.H
	$(CXX) $(FLAGS) $(INCLUDE) $@.C -o $@ $(LIBS)

test-load-productos: test-load-productos.C tablas.H grafo.H net.H
	$(CXX) $(FLAGS) $(INCLUDE) $@.C -o $@ $(LIBS)

test-load-productores: test-load-productores.C tablas.H grafo.H net.H
	$(CXX) $(FLAGS) $(INCLUDE) $@.C -o $@ $(LIBS)

load-net: load-net.C tablas.H grafo.H net.H
	$(CXX) $(FLAGS) $(INCLUDE) $@.C -o $@ $(LIBS)

net-lex.C: net-lex.lex net-parser.H net.tab.c
	${FLEX} -o net-lex.C net-lex.lex 

net-lex.o: net-lex.C
	$(CXX) $(FLAGS) $(INCLUDE) -c net-lex.C

test-lex: test-lex.C net-lex.C 
	$(CXX) $(FLAGS) $(INCLUDE) $@.C -o $@ -lfl -lreadline $(LIBS)

net.tab.c: net.y net-parser.H
	$(YACC) -d --report=all net.y

net.tab.o: net.tab.c
	$(CXX) $(FLAGS) $(INCLUDE) -c net.tab.c

repl: repl.C tablas.H grafo.H net.H net-lex.C net.tab.o
	$(CXX) $(FLAGS) $(INCLUDE) $@.C -o $@ net.tab.o -lreadline $(LIBS)

test-net: test-net.C tablas.H grafo.H net.H
	$(CXX) $(FLAGS) $(INCLUDE) $@.C -o $@ $(LIBS)

net: net.tab.c net-lex.o
	$(CXX) $(FLAGS) $(INCLUDE) net.tab.c -o net -lfl -lreadline $(LIBS)
#	$(CXX) $(FLAGS) $(INCLUDE) net.tab.c -o net net-lex.o -lfl -lreadline $(LIBS)

test: test.lex test.y net-parser.H net-symtbl.H net-tree.H $(OBJ)
	$(FLEX) -o test.C test.lex 
	$(YACC) -d -t --report-=all test.y
	$(CXX) $(FLAGS) $(INCLUDE) -c test.tab.c
	$(CXX) $(FLAGS) $(INCLUDE) test.C -o test test.tab.o $(OBJ) -lreadline $(LIBS)