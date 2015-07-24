ALEPH = ~/aleph-w

CLANGPATH = /home/lrleon/LLVM-3.6.1/bin

CXX = $(CLANGPATH)/clang++
CC = $(CLANGPATH)/clang

WARN = -Wall -Wextra -Wcast-align -Wno-sign-compare -Wno-write-strings \
       -Wno-parentheses -Wno-invalid-source-encoding

FLAGS = -g -O0 -std=c++14 $(WARN)

OPT=-O3 -DWITHOUT_NANA -DNDEBUG -std=c++14 $(WARN)

INCLUDE = -I. -I $(ALEPH) 

LIBS = -L $(ALEPH) \
       -lAleph -lnana -lm -lgsl -lgslcblas -lgmp -lmpfr -lasprintf -lpthread -lc

all: test test-1 test-2 test-3 test-io transform-data transform-data-op

clean:
	rm -f test testcsv test-1 test-2 test-3 test-1-op test-2-op test-3-op test-io transform-data transform-data-op transform-data-2 transform-data-2-op

csvparser.o: csvparser.h csvparser.c
	$(CC) $(INCLUDE) $*.c -g -O0 -c

test: test.C csvparser.o tablas.H grafo.H
	$(CXX) $(FLAGS) $(INCLUDE) $@.C -o $@ csvparser.o $(LIBS)

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

