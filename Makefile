# CC = Compiler
CC=g++

# CFLAGS = CompilatorFLAGS
CFLAGS=-c

all: lang

lang: ast_comp asm_comp

# helping libs
lex_anal.o: Libs/LexemAnalyzer/lex_anal.cpp
	$(CC) $(CFLAGS) Libs/LexemAnalyzer/lex_anal.cpp -o Obj/lex_anal.o

bin_tree.o: Libs/BinaryTree/bin_tree.cpp
	$(CC) $(CFLAGS) Libs/BinaryTree/bin_tree.cpp    -o Obj/bin_tree.o

tokens.o:   Libs/Tokens/tokens.cpp
	$(CC) $(CFLAGS) Libs/Tokens/tokens.cpp          -o Obj/tokens.o
# AST_COMPILING
ast_comp: ast_main.o ast_comp.o lex_anal.o bin_tree.o tokens.o
	$(CC) Obj/ast_main.o Obj/ast_comp.o Obj/lex_anal.o Obj/bin_tree.o Obj/tokens.o -o Bin/ast_comp


ast_main.o: Libs/AST/ast_main.cpp
	$(CC) $(CFLAGS) Libs/AST/ast_main.cpp           -o Obj/ast_main.o

ast_comp.o: Libs/AST/ast_comp.cpp
	$(CC) $(CFLAGS) Libs/AST/ast_comp.cpp           -o Obj/ast_comp.o

# ASM_COMPILING
asm_comp: asm_main.o asm_comp.o ast_comp.o lex_anal.o bin_tree.o tokens.o
	$(CC) Obj/asm_main.o Obj/asm_comp.o Obj/ast_comp.o Obj/lex_anal.o Obj/bin_tree.o Obj/tokens.o -o Bin/asm_comp


asm_main.o: Libs/Asm/asm_main.cpp
	$(CC) $(CFLAGS) Libs/Asm/asm_main.cpp           -o Obj/asm_main.o

asm_comp.o: Libs/Asm/asm_comp.cpp
	$(CC) $(CFLAGS) Libs/Asm/asm_comp.cpp           -o Obj/asm_comp.o

# cleaning
clean:
	rm -rf Obj/* Bin/*
