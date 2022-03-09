#include <stdio.h>
#include <stdlib.h> /*atoi*/
#include <unistd.h> /* write */
#include <time.h>

void gen_key(int);

int main(int argc, char** argv)
{
	srand(time(NULL));
	
	if(argc > 1)
	{
		int keylen = atoi(argv[1]);
		gen_key(keylen);
	}
	
return EXIT_SUCCESS;
}

void gen_key(int keylen)
{
	char written[keylen]; /* track already-used characters */
	
	char k[1];
	
	for(int i = 0; i < keylen; i++)
	{
		int k_int = rand() % 27 + 1; // Generate a number from 1 - 27
		if(k_int == 27) /* 27 is associated with space */
			k[0] = 32;
		else 			/* Otherwise, shift k_int to appropriate ASCII (65-90) */
			k[0] = k_int + 64;
		
		write(STDOUT_FILENO, k, 1);
	}
	write(STDOUT_FILENO, "\n", 1);
}