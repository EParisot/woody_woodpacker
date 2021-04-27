int main()
{
	__asm__("mov $0x42424242, %rax");
	__asm__("jmp %rax");
}