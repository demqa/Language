# Language

## Abstract Syntax Tree Standard
> AST Standard was developed by a group of people, all information is here: https://github.com/futherus/Language/blob/master/tree_standard.md


## My Language

This is my musical language (Italiano). Now I will improve some things & then I will translate my language in my x86-64 architecture, and then I will do executable file (ELF-64).


## Translation

There I want to prepare to translate my Abstract Syntax Tree into x86-64 binary code.

I have to solve some problems with my compilator:
1. How to store variables in differents functions with the same name in compilation time?
   a. Easy. It is not a problem. Lol. I'm stupid.
2. Do I want to emulate stack when executing?
   a. answer: no
   b. On the first version I want to reduce number of variables to 6 to simplify work with stack.
3. What registers for what I will use?
   a. RDI, RSI, RCX, RDX, R8, R9 - parameters
      A. Okay, I understand, that this idea was really bad. So I will use stack to store all parameters, and local variables.
   b. RAX - main accumulator. It there will be more than 3 parameters I have to save RDX somewhere not to 
4. Where will be my RAM? And how can I allocate memory? (I think this is too hard for now).
   a. I found out, that RAM is not necessary for my project. There I can store some constants, that are global in file. But using global variables is harmless for security, so I will avoid it, and my users will be secured from it.


# How to use

There are code samples in *code* directory. Default file of source code in my language is code.

## Compilation with NASM

You can use my NASM logs to compile project with NASM and link with ld.

File *asm_log* is created with my own compiler. 

```shell
demqa:code/ (x86-64*) $ nasm -felf64 asm_log -o fact.o
demqa:code/ (x86-64*) $ nasm -felf64 ../src/ASMFuncs/print.asm -o print.o
demqa:code/ (x86-64*) $ nasm -felf64 ../src/ASMFuncs/scan.asm -o scan.o
demqa:code/ (x86-64*) $ ld fact.o scan.o print.o
demqa:code/ (x86-64*) $ ./a.out
5
120
```

