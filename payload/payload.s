global _start

section .text

_start:
  mov rbx, rax

  push 87
  mov rax, 1        ; write(
  mov rdi, 1        ;   STDOUT_FILENO,
  mov rsi, rsp      ;   "W",
  mov rdx, 1   		;   sizeof(char)
  syscall           ; );
  
  push 111
  mov rax, 1
  mov rdi, 1
  mov rsi, rsp
  mov rdx, 1
  syscall

  push 111
  mov rax, 1
  mov rdi, 1
  mov rsi, rsp
  mov rdx, 1
  syscall

  push 100
  mov rax, 1
  mov rdi, 1
  mov rsi, rsp
  mov rdx, 1
  syscall

  push 121
  mov rax, 1
  mov rdi, 1
  mov rsi, rsp
  mov rdx, 1
  syscall

  push 10
  mov rax, 1
  mov rdi, 1
  mov rsi, rsp
  mov rdx, 1
  syscall

  push rbx
  
  ;mov rax, 60      ; exit(
  ;mov rdi, 0       ;   EXIT_SUCCESS
  ;syscall          ; );