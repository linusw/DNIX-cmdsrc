/*
 * Kill_Command
 * Send Signal to Process
 *
 *	Copyright (C) DIAB T{by 1983
 *	Written by: Per-Arne Svensson 
 */

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include "../cmd_err.h"

main(argc,argv)
int	argc;				/* Command_line argument count  */
char	*argv[];			/* Array of strings (cmd parms) */
{
	extern	int errno;
	extern	char *sys_errlist[];	/* Array of strings (err msgs)  */
	register	signo;		/* Signal number		*/
	register	pid;		/* Process ID			*/
	register	res;		/* Result of system call	*/
	int	errlev;			/* Return status    (err nmbr)  */
	int	i;			/* Loop variable		*/
	char	c;			/* Loop invariant		*/

	/*	Initiate prompt string					*/
	PRMPT = *argv;

	errlev = 0;			/* Set return status to OK	*/
	argc--;				/* Don't count first argument	*/
	argv++;

	/* Test if any prameters are given to the command		*/

	if	(!argc)
	{
usage:
		fprintf(stderr, "usage: %s [ -signo ] pid ...\n", PRMPT);
		exit(2);
	}

	/* Test if any switches are defined (only one is allowed)	*/

	if	(*argv[0] == '-')
	{
		if (!isdigit(argv[0][1])) goto usage;
		signo = atoi(&argv[0][1]);
		argv++;
		argc--;
	}

	/* Default value for switch 'signo' is SIGTERM			*/

	else	signo = SIGTERM;

	/* While any arguments, do send kill_message to process		*/

	if	(!argc) goto usage;
	while	(argc)
	{

		/* Test if the pid is a valid number			*/

		i = 0;
		while	(c=argv[0][i++])
		{
			if	(!isdigit(c))
				goto usage;	/* Error bad Process_ID */
		}

		/* Make a kill system_call */

		res =	kill(pid = atoi(*argv), signo);

		/* Test if any errors occurred */

		if	( res < 0 )
		{
			fprintf(stderr,"%u: %s\n", pid, sys_errlist[errno]);
			errlev = 1;
		}
		argc--;
		argv++;
	}
	return(errlev);
}


