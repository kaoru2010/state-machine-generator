all: smc_compiler

smc_compiler: smc_compiler_parser.c smc_compiler_lexer.c
	gcc -o smc_compiler -Wall smc_compiler_parser.c smc_compiler_lexer.c

smc_compiler_parser.c: smc_compiler_parser.y
	./lemon -p -s smc_compiler_parser.y

smc_compiler_lexer.c: smc_compiler_lexer.l smc_compiler_parser.c
	flex --outfile=smc_compiler_lexer.c smc_compiler_lexer.l

