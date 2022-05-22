[bits 64]

    SECTION .text

print:

    mov r10, rsi

    mov rdi, r10
    mov rbx, rax

    call Itoa10

    mov rsi, r10
    mov rax, 1
    mov rdi, 1
    mov rdx, 10
    syscall

    ret

%define ZERO  '0'
%define NINE  '9'
%define MINUS '-'
%define PLUS  '+'

%define END_OF_STRING 10

%define TEN 10

;------------------------------------------------
; Entry:
; RDI - output buffer
; RBX - input integer
; Destr: AX, DX, CX
;------------------------------------------------
Itoa10:

       mov rcx, TEN

       cmp rbx, 0h
       push rdi
       jns .proceed

       mov al, MINUS
       stosb

       pop rdx
       push rdi

       neg rbx

.proceed:

       xor rdx, rdx

       mov rax, rbx   ; ax = N
       div rcx        ; ax = N / 10

       mov rbx, rax   ; saving next integer

       mov rax, rdx   ; ax = N % 10

       add al, ZERO

       stosb

       cmp bx, 0
       jne .proceed


       mov al, END_OF_STRING
       stosb

       pop rbx
       sub rdi, 2

.reverse:
       mov al, [rdi]
       mov dl, [rbx]
       mov [rdi], dl
       mov [rbx], al

       inc rbx
       dec rdi

       cmp rbx, rdi
       jb .reverse

       ret
;------------------------------------------------