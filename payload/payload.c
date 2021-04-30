void _start()
{
	__asm__(

	"push %rax \n"			 // save used registers
	"push %rdi \n"
	"push %rsi \n"
	"push %rdx \n"
	"push %r9 \n"
	"push %r11 \n"
	);



	int a = 21;
	int b = 21;
	int c = a + b;
	int tab[2];

	for (int i = 0; i < 2; ++i)
		tab[i] = 42;
	
	__asm__(
	"pop %r11 \n"				 // restore used registers
	"pop %r9 \n"
	"pop %rdx \n"
	"pop %rsi \n"
	"pop %rdi \n"
	"pop %rax \n"

	"push $0x42424242 \n"	 // jump back to entrypoint
	"ret \n"
	);
}