# CC = Compiler
CC=g++

# CFLAGS = CompilatorFLAGS
CFLAGS=-c

all: lang

lang: ast_comp asm_comp

bin_tree.o: Libs/BinaryTree/bin_tree.cpp
	$(CC) $(CFLAGS) Libs/BinaryTree/bin_tree.cpp    -o Obj/bin_tree.o


ast_comp: ast_main.o lex_anal.o bin_tree.o ast_comp.o
	$(CC) Obj/ast_main.o Obj/lex_anal.o Obj/bin_tree.o Obj/ast_comp.o -o ast_comp


ast_main.o: Libs/AST/ast_main.cpp
	$(CC) $(CFLAG) Libs/AST/ast_main.cpp            -o Obj/ast_main.o

ast_comp.o: Libs/AST/ast_comp.cpp
	$(CC) $(CFLAG) Libs/AST/ast_comp.cpp            -o Obj/ast_comp.o

lex_anal.o: Libs/LexemAnalyzer/lex_anal.cpp
	$(CC) $(CFLAGS) Libs/LexemAnalyzer/lex_anal.cpp -o Obj/lex_anal.o


asm_comp: asm_main.o asm_comp.o bin_tree.o
	$(CC) Obj/asm_main.o Obj/asm_comp.o Obj/bin_tree.o -o asm_comp


asm_main.o: Libs/Asm/asm_main.o
	$(CC) $(CFLAGS) Libs/Asm/asm_main.cpp           -o Obj/asm_main.o

asm_comp.o: Libs/Asm/asm_comp.o
	$(CC) $(CFLAGS) Libs/Asm/asm_comp.cpp           -o Obj/asm_comp.o



clean:
	rm -rf *.o lang
