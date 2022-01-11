/*
 * sleep - command
 *
 *	Copyright (C) DIAB T{by 1983
 *	Written by: Per-Arne Svensson 
 */

/*
 *	suspend execution for an interval
 *	sleep time
 */
/*	Per-Arne Svensson DIAB T{by			*/

#include <stdio.h>
#include "../cmd_err.h"

/*	Main Program							*/
/*	============							*/
main(argc, argv)
register char	*argv[];
register int	argc;
{
	register unsigned int i;

	/*	Initiate prompt string					*/
	PRMPT = *argv;

	++argv;
	--argc;

	if(argc == 1 && isnumber(*argv) && (i = atoi(*argv)) <= 65535){
		sleep(i);
	}else {
		fprintf(stderr,"usage: %s time\n", PRMPT);
		exit(1);
	}
}
