/*  Type own tty name */

#include <stdio.h>
char	*ttyname();

main(count,param)

int count;
char *param[];
{
	char  *namepointer;

	if (namepointer=ttyname(0))
	{
		printf("%s\n",namepointer);
		exit(0);
	}
	else
	{
		printf("%s\n","not a tty");
		exit(1);
	}
}
