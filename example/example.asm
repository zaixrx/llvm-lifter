section .text
global _start

_start:
    mov al, 0x42
    mov ax, 0x1234
    mov eax, 0x12345678
    mov rax, 0x1122334455667788
    mov bl, al
    mov bx, ax
    mov ebx, eax
    mov rbx, rax
    mov r9, r8
    mov cl, [rsi + 16]
    mov cx, [rsi + 18]
    mov ecx, [rsi + 20]
    mov rcx, [rsi + 24]

    mov byte [rsi], 0x42
    mov word [rsi + 2], 0x1234
    mov dword [rsi + 4], 0x12345678
    mov qword [rsi + 8], -1
    mov [rsi + 16], al
    mov [rsi + 18], ax
    mov [rsi + 20], eax
    mov [rsi + 24], rax

    mov edx, 2
    mov [rsi + rdx*8 + 16], rax
    mov r10, [rsi + rdx*8 + 16]

    mov [rel buffer], rax
    mov r11, [rel buffer]

section .bss
align 8
buffer: resb 64
