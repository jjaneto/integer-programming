# Copyright 2017, Gurobi Optimization, Inc.

PLATFORM = mac64
INC      = /Library/gurobi752/mac64/include/
CC       = gcc
CPP      = g++
CARGS    = -m64 -g
CLIB     = -L/Library/gurobi752/mac64/lib -lgurobi75
RELEASE := $(shell uname -r | cut -f 1 -d .)
CPPLIB   = -L/Library/gurobi752/mac64/lib -lgurobi_c++ -lgurobi75

all: ordinary_formulation_c++

run: run_c++

run_c++: run_ordinary_formulation_c++ 

%_c++: ./%_c++.cpp
	$(CPP) $(CARGS) -o $@ $< -I$(INC) $(CPPLIB) -lm

run_ordinary_formulation_c++: ordinary_formulation
	./ordinary_formulation

clean:
	rm -rf *.o *_c *_c++ *.class *.log *.rlp *.lp *.bas *.ilp *.mps *.dSYM *~
