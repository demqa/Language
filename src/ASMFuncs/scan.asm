[bits 64]

    SECTION .text

scan:

    mov r10, rsi

    mov rax, 0     ; read
    mov rdi, 0     ; stdin
    mov rsi, r10
    mov rdx, 10    ;
    syscall

    call Atoi10

    ret


%define ZERO  '0'
%define NINE  '9'
%define MINUS '-'
%define PLUS  '+'

%define END_OF_STRING 10

;------------------------------------------------
; Entry:
; RSI - start index of string, containing [0-9] or -(+) in the beggining
; Out:
; RAX - result integer (signed)
; Destr: BX, DX, CL
;------------------------------------------------
Atoi10:
       xor bx, bx
       xor ah, ah
       xor cl, cl

       lodsb

       cmp al, MINUS
       je .negative

       cmp al, PLUS
       je .positive

       cmp al, END_OF_STRING
       je .ret

       cmp al, ZERO ; '0'
       jb .ret
       cmp al, NINE ; '9'
       ja .ret

       sub al, ZERO
       xchg bl, al

       jmp .positive

.negative:
       inc cl

.positive:

       lodsb

       cmp al, END_OF_STRING
       je .exit

       cmp al, ZERO
       jb .ret
       cmp al, NINE
       ja .ret

       sub al, ZERO

       mov rdx, rbx         ; MULTIPLYING
                            ; BY 10
       sal rbx, 2           ;
       add rbx, rdx         ;
       sal rbx, 1           ;

       add rbx, rax

       jmp .positive

.exit:
       cmp cl, 0h
       jz .ret

       neg rbx

.ret:
       mov rax, rbx
       ret
;------------------------------------------------
