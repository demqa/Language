
    SECTION .text

GLOBAL _start

_start:

    call scan

    mov rax, 0x3C
    xor rdi, rdi
    syscall


scan:

    mov rax, 0     ; read
    mov rdi, 0     ; stdin
    mov rsi, buff  ; buff_ptr
    mov rdx, 10    ;
    syscall

    ret

    SECTION .data

buff:
    db 10 dup(0)
    db 0
