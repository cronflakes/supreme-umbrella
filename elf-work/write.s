global _start

section .text 

_start: 

	mov rbp, rsp
	mov rax, 0x0a21212121216427
	push rax
	mov rax, 0x3331623474736e75
	push rax
	
	xor rax, rax
	mov al, 1
	xor rdi, rdi
	mov rdi, 1
	lea rsi, [rbp - 16]	
	mov rdx, 14
	syscall

	mov rax, 60
	mov rdi, 0
	syscall


# nasm -f elf64 -o <relocatable> %.s
# ld -o <executable> <relocatable>

