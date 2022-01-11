/*
 * mkdir file ...
 *
 *	Copyright (C) DIAB T{by 1983
 *	Written by: Per-Arne Svensson 
 */

/*
 *	make one or more directories
 */
/*	Per-Arne Svensson DIAB T{by			*/

#include <stdio.h>
#include "../cmd.h"
#include "../cmd_err.h"

/*	External functions						*/
/*	==================						*/
int	makedir();
struct	entry *makentry();	/*	Make one tree entry		*/
struct	entry *freetree();	/*	Free memory occupied by tree	*/

/*	main program							*/
/*	============							*/
main(argc, argv)
register int argc;
register char *argv[];

{
int	result = 0;
struct	entry	*root;		/*	Directory file entry		*/

	/*	Initiate prompt string					*/
	PRMPT = *argv;

	--argc;
	++argv;

	if (geteuid() != 0) {	/*	No superuser-permission		*/
		fprintf(stderr, SUONLY);
		exit(1);
	}

	/*	Test if enough number of arguments			*/
	if (argc < 1) {
		fprintf(stderr, "usage: %s file ...\n", PRMPT);
		exit(1);
	}

	while (*argv) {
		result |= makedir((root = makentry(*argv++, "")));
		freetree(root);
	}
	exit(result);
}
