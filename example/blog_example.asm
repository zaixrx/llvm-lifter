bits 64
section .text
global _start

_start:
cmp rax, 0
jz true_branch

add rax, 1
jmp merge

true_branch:
add rax, 2

merge:
sub rax, 1
jnz merge

ret
