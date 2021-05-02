#include <stdio.h>

int
main(int argc, char *argv[]) {
	printf("%d args\n", argc);
	if (argc == 2)
		printf("%s\n", argv[1]);
	return (0x0);
}