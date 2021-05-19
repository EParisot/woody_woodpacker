#include <stdio.h>

int uninit;
char init[5] = "test\0";

int main(void) {
	printf("Hello, World!\n");
	uninit++;
	init[0] = 42;
	printf("tests: %d, %s\n", uninit, init);
	return (0x0);
}
