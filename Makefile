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

all: test

clean:
	rm -f test testcsv

csvparser.o: csvparser.h csvparser.c
	$(CC) $(INCLUDE) $*.c -g -O0 -c

test: test.C csvparser.o common.H
	$(CXX) $(FLAGS) $(INCLUDE) $@.C -g -O0 -o $@ csvparser.o $(LIBS)

testcsv: testcsv.C csvparser.o
	$(CXX) $(FLAGS) $(INCLUDE) $@.C -g -O0 -o $@ csvparser.o $(LIBS)
