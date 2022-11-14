.globl main
main:
push %rbp
movq %rsp, %rbp
movq $0, %rax
push %rax
movq $1, %rax
push %rax
movq $1, %rax
pop %rcx
addq %rcx, %rax
movq %rax, -8(%rbp)
movq -8(%rbp), %rax
movq %rbp, %rsp
pop %rbp
ret
