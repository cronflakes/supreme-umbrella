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
	mov sil, 1
	lea rsi, [rbp - 16]	
	xor rdx, rdx
	mov dl, 16
	syscall

	mov rax, 60
	xor rdi, rdi
	syscall


# nasm -f elf64 -o <relocatable> %.s
# ld -o <executable> <relocatable>

