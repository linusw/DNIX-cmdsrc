main(argc,argv)
int argc;
char *argv[];
{
	int ant,nr;
	register char *p;

	p = argv[1];
	if (argc != 2)
		printf("Wrong nr of arguments\n");
	else
	{
		ant=0;
		nr=0;
		while (*p != '\0' )
		{
			if (*p == '/' )
				nr=ant;
			p++;
			ant=ant+1;
		}
		if (nr == 0)
			printf(".\n");
		else
		{
			write(1,argv[1],nr);
			printf("\n");
		}

	}
}
