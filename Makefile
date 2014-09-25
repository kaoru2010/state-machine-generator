all: smc_compiler

CC=gcc
CXX=g++
CFLAGS=-I.
CXXFLAGS=-I. -std=c++11
DEPS = smc_compiler.h smc_compiler_parser.h gen.h

# Disable built-in rules.
# See
#   make -p -f /dev/null
%.c: %.y
%.c: %.l

%.o: %.c $(DEPS)
	$(CC) -Wall -c -o $@ $< $(CFLAGS)

%.o: %.cpp $(DEPS)
	$(CXX) -Wall -std=c++11 -c -o $@ $< $(CXXFLAGS)

%.cpp: %.y lemon
	./lemon -X -p -s $<

smc_compiler: smc_compiler_parser.o smc_compiler_lexer.o smc_compiler.o gen_javascript.o gen_swift.o
	$(CXX) -o smc_compiler smc_compiler_parser.o smc_compiler_lexer.o smc_compiler.o gen_javascript.o gen_swift.o

smc_compiler_lexer.cpp: smc_compiler_lexer.l smc_compiler_parser.cpp
	flex --outfile=smc_compiler_lexer.cpp smc_compiler_lexer.l

lemon: lemon.c
	$(CC) -o lemon -O2 lemon.c

clean:
	rm -f smc_compiler smc_compiler_lexer.cpp smc_compiler_parser.cpp smc_compiler_parser.h
