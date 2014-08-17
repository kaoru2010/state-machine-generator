all: smc_compiler

CC=gcc
CXX=g++
CFLAGS=-I.
CXXFLAGS=-I.
DEPS = smc_compiler.h smc_compiler_parser.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

%.o: %.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

smc_compiler: smc_compiler_parser.o smc_compiler_lexer.o smc_compiler.o
	g++ -o smc_compiler smc_compiler_parser.o smc_compiler_lexer.o smc_compiler.o

smc_compiler_parser.c: smc_compiler_parser.y lemon
	./lemon -p -s smc_compiler_parser.y

smc_compiler_lexer.c: smc_compiler_lexer.l smc_compiler_parser.c
	flex --outfile=smc_compiler_lexer.c smc_compiler_lexer.l

lemon: lemon.c
	gcc -o lemon -O2 lemon.c
