# include <stdio.h>
# include <stdlib.h>
# include <strings.h>

#define KEY "shqs8N674çzqiàis"
#define IV "gs6SHzèf"

int main(int ac, char *av[])
{
	if (ac != 2)
	{
		printf("Error: no input provided\n");
		return 0;
	}
	printf("Encrypting '%s'...\n", av[1]);

	unsigned int 	X[8];
	unsigned int 	C[8];
	short			K[16];
	unsigned int 	b = 0;

	bzero(X, 8 * sizeof(unsigned int));
	bzero(C, 8 * sizeof(unsigned int));
	bzero(K, 8 * sizeof(short));

	for (int i = 0; i < 16; ++i)
	{
		if (i % 2 == 0)
			K[i/2] = (KEY[i] << 8) + KEY[i+1];
	}

	for (int j = 0; j < 8; ++j)
	{
		if (j % 2 == 0)
		{
			X[j] = K[(j + 1) % 8] | K[j];
			C[j] = K[(j + 4) % 8] | K[(j + 5) % 8];
		}
		else
		{
			X[j] = K[(j + 5) % 8] | K[(j + 4) % 8];
			C[j] = K[j] | K[(j + 1) % 8];
		}
	}

	printf("\n");
	return 0;
}