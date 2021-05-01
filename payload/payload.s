global _start

section .text

_start:

	push rax			; // save used registers
	push rdi
	push rsi
	push rdx
	push r9
	push r11

	;// TODO decrypt text
	

	mov rax, qword "...."
	push rax
	mov rax, 1       	 ; //write(
	mov rdi, 1       	 ; //  STDOUT_FILENO,
	mov rsi, rsp     	 ; //  "Woody",
	mov rdx, 5   	  	 ; //  sizeof(char)
	syscall        		 ; //);
	pop rax				 ; //clear stack

	mov rax, qword "WOODY"
	push rax
	mov rax, 1
	mov rdi, 1
	mov rsi, rsp
	mov rdx, 5
	syscall
	pop rax

	mov rax, qword "...."
	push rax
	mov rax, 1
	mov rdi, 1
	mov rsi, rsp
	mov rdx, 4
	syscall
	pop rax
	
	mov rax, 10			 ; // print \n
	push rax
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

	mov rax, 0x42424242	; // jump back to entrypoint
	jmp rax

	;mov rax, 60
	;mov rdi, 0
	;syscall