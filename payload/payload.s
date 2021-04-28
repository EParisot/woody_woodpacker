global _start

section .text

_start:

	push rax			; // save used registers
	push rdi
	push rsi
	push rdx
	push r9
	push r11

	push 10
	push 121
	push 100
	push 111
	push 111
	push 87

	mov rax, 1       	 ; //write(
	mov rdi, 1       	 ; //  STDOUT_FILENO,
	mov rsi, rsp     	 ; //  "W",
	mov rdx, 1   	  	 ; //  sizeof(char)
	syscall        		 ; //);
	pop rax				 ; //clear stack
	
	mov rax, 1
	mov rdi, 1
	mov rsi, rsp
	mov rdx, 1
	syscall
	pop rax
	
	mov rax, 1
	mov rdi, 1
	mov rsi, rsp
	mov rdx, 1
	syscall
	pop rax

	mov rax, 1
	mov rdi, 1
	mov rsi, rsp
	mov rdx, 1
	syscall
	pop rax

	mov rax, 1
	mov rdi, 1
	mov rsi, rsp
	mov rdx, 1
	syscall
	pop rax
	
	mov rax, 1
	mov rdi, 1
	mov rsi, rsp
	mov rdx, 1
	syscall
	pop rax

	pop r11				; // restore used registers
	pop r9
	pop rdx
	pop rsi
	pop rdi
	pop rax