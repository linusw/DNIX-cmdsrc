/*  basename.c  */

/*
 *  This file contains the source code for the basename command.
 */

/*  1984-03-16	Peter Andersson  */

main(argc, argv)
register	int	argc;		/*  argument count  */
register	char	**argv;		/*  pointers to arguments  */
{
	register	char	*name;	/*  used to skip all slashes  */
	register	char	*ip;	/*  1st arg pointer index  */
	register	char	*ipsave;	/*  save ip here  */
	register	char	*sp;	/*  suffix pointer index  */

	if (argc > 1) {			/*  if any arguments  */
		for (ip = name = *(++argv) ; *ip != '\0' ; ip++)
			if (*ip == '/')
				name = ip + 1;
		if (argc > 2) {			/*  if any suffix  */
			for (sp = *(++argv) ; *sp != '\0' ; sp++)
				;
			ipsave = ip;
			while (sp > *argv && *(--sp) == *(--ip))
				;
			if (*sp != *ip)
				ip = ipsave;
		}				/*  if (argc > 2)  */
		if (name < ip)			/*  if anything left  */
			write(1, name, (int) (ip - name));
	}					/*  if (argc > 1)  */
	write(1, "\n", 1);
	exit(0);
}
