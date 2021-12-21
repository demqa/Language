# CC = Compiler
CC=g++

# CFLAGS = CompilatorFLAGS
CFLAGS=-c -g -std=c++14 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++0x-compat -Wc++11-compat -Wc++14-compat -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlarger-than=8192 -Wlogical-op -Wmissing-declarations -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstack-usage=8192 -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -fcheck-new -fsized-deallocation -fstack-check -fstack-protector -fstrict-overflow -fchkp-first-field-has-own-bounds -fchkp-narrow-to-innermost-array -flto-odr-type-merging -fno-omit-frame-pointer -fPIE -lm -pie

all: lang

lang: ast_comp asm_comp processor ast_to_lang launch

# helping libs
lex_anal.o: Libs/LexemAnalyzer/lex_anal.cpp
	$(CC) $(CFLAGS) Libs/LexemAnalyzer/lex_anal.cpp -o Obj/lex_anal.o

bin_tree.o: Libs/BinaryTree/bin_tree.cpp
	$(CC) $(CFLAGS) Libs/BinaryTree/bin_tree.cpp    -o Obj/bin_tree.o

tokens.o:   Libs/Tokens/tokens.cpp
	$(CC) $(CFLAGS) Libs/Tokens/tokens.cpp          -o Obj/tokens.o
list.o:     Libs/List/list.cpp
	$(CC) $(CFLAGS) Libs/List/list.cpp              -o Obj/list.o

# AST_COMPILING
ast_comp: ast_main.o ast_comp.o lex_anal.o bin_tree.o tokens.o
	$(CC) Obj/ast_main.o Obj/ast_comp.o Obj/lex_anal.o Obj/bin_tree.o Obj/tokens.o -o Bin/ast_comp


ast_main.o: Libs/AST/ast_main.cpp
	$(CC) $(CFLAGS) Libs/AST/ast_main.cpp           -o Obj/ast_main.o

ast_comp.o: Libs/AST/ast_comp.cpp
	$(CC) $(CFLAGS) Libs/AST/ast_comp.cpp           -o Obj/ast_comp.o

# ASM_COMPILING
asm_comp: asm_main.o asm_comp.o ast_comp.o lex_anal.o bin_tree.o tokens.o list.o
	$(CC) Obj/asm_main.o Obj/asm_comp.o Obj/ast_comp.o Obj/lex_anal.o Obj/bin_tree.o Obj/tokens.o Obj/list.o -o Bin/asm_comp

asm_main.o: Libs/Asm/asm_main.cpp
	$(CC) $(CFLAGS) Libs/Asm/asm_main.cpp           -o Obj/asm_main.o

asm_comp.o: Libs/Asm/asm_comp.cpp
	$(CC) $(CFLAGS) Libs/Asm/asm_comp.cpp           -o Obj/asm_comp.o


processor: asm cpu

asm: asm.o
	$(CC) Obj/asm.o -o Bin/asm

cpu: cpu.o stack.o
	$(CC) Obj/cpu.o Obj/stack.o -o Bin/cpu

asm.o:
	$(CC) $(CFLAGS) Libs/CPU/asm.cpp                -o Obj/asm.o

cpu.o:
	$(CC) $(CFLAGS) Libs/CPU/cpu.cpp                -o Obj/cpu.o

stack.o:
	$(CC) $(CFLAGS) Libs/CPU/stack/stack.cpp        -o Obj/stack.o

launch: launch.o
	$(CC) Obj/launch.o -o launch

launch.o:
	$(CC) $(CFLAGS) launch.cpp -o Obj/launch.o


ast_to_lang: ast_tl_main.o ast_to_lang.o asm_comp.o ast_comp.o lex_anal.o bin_tree.o tokens.o list.o
	$(CC) Obj/ast_tl_main.o Obj/ast_to_lang.o Obj/asm_comp.o Obj/ast_comp.o Obj/lex_anal.o Obj/bin_tree.o Obj/tokens.o Obj/list.o -o Bin/ast_to_lang

ast_tl_main.o: Libs/ASTLang/ast_tl_main.cpp
	$(CC) $(CFLAGS) Libs/ASTLang/ast_tl_main.cpp  -o Obj/ast_tl_main.o

ast_to_lang.o: Libs/ASTLang/ast_to_lang.cpp
	$(CC) $(CFLAGS) Libs/ASTLang/ast_to_lang.cpp  -o Obj/ast_to_lang.o

# cleaning
clean:
	rm -rf Obj/* Bin/*
