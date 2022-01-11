/*
 * 	nice	command
 *	Change Running Priority of Process
 *
 *	Copyright (C) DIAB T{by 1983
 *	Written by: Per-Arne Svensson 
 */

#include <stdio.h>
#include "../cmd_err.h"

main(argc, argv)
int	argc;				/* Command_line argument count	*/
char	*argv[];			/* Command_line paramenters	*/
{

	int	nicarg = 10;		/* Priority (Default = 10)	*/
	extern	int	errno;		/* Error number from syst_call	*/

	/*	Initiate prompt string					*/
	PRMPT = *argv;

	argc--;				/* Don't count first argument	*/
	argv++;				/* Skip over first argument	*/

	/* Test if the priority is specified (-n)	*/
	if	(argc && **argv == '-' )
	{	
		nicarg = atoi(&argv[0][1]);
		argc--;
		argv++;
	}

	/* Test if any process number is given		*/
	if	(!argc)
	{
		fprintf(stderr,"usage: %s [ -n ] command\n", PRMPT);
		exit(1);
	}

	/* Set running priority of this process		*/
	nice(nicarg);

	/* Execute the specified process (wiping out this one)	*/
	execvp(*argv,argv);

	/* Error, the execute did not succeed			*/
	perror(*argv);
	exit(1);
}
