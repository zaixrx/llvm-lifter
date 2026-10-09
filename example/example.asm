; NOTE: code isn't made to run

bits 64
default abs
section .text
global _start

; Representative lifting cases; guest memory and runtime hooks are external.
_start:
    mov rsp, 0x20000
    mov rsi, 0x10000

    ; Full-width write, partial writes, high byte, and 32-bit zero extension.
    mov rax, 0x8877665544332211
    mov ax, 0x1234
    mov ah, 0x80
    mov eax, 0x80000000
    mov rbx, rax

    ; Memory widths and representative effective-address forms.
    mov byte [rsi], 1
    mov al, [rsi]
    mov word [rsi + 2], 3
    mov ax, [rsi + 2]
    mov dword [rsi + 4], -1
    mov eax, [rsi + 4]
    mov edx, 2
    mov [rsi + rdx*8 - 8], rax
    mov rbx, [rsi + rdx*8 - 8]
    mov [rel scratch], rax
    mov rbx, [rel scratch]
    mov al, [gs:esi + edx*4]
    mov al, [0]

    ; Logical handlers: widths, memory operands, zero/sign/parity results.
    and al, 0x80
    or ax, 3
    xor eax, eax
    and rbx, -1
    or dword [rsi + 4], eax
    xor rbx, [rsi + 8]

    ; CALL forms; the lifter delegates targets to its runtime hook.
    call plain_return
    call rbx
    call [rsi + 8]

    ; Both JE edges are visited; MOV preserves the flags from XOR.
    xor eax, eax
    je plain_return
    mov eax, 1
    or eax, eax
    je adjusted_return
    jmp indirect_jump

plain_return:
    ret
adjusted_return:
    ret 16
indirect_jump:
    jmp [rsi + 8]

scratch:
    dq 0
