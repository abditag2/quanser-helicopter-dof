#include "q8user-betti.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main()
{
	int i;
	int tmp;
	char ted;
	while (1)
	{
		tmp = get_enc(1);
		printf("Encoder read %x\n",tmp);
		printf("type to continue ctrl+c to quit\n");
		ted = getchar();
	/*	if (ted != 'a')
			return 0;
		while(ted != EOF)
			ted = getchar();*/
	}
	return 0;
}
