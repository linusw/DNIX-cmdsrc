/*
 *	chgrp gid file ...
 *
 *	Copyright (C) DIAB T{by 1983
 *	Written by: Per-Arne Svensson 
 */

/*
 *	change group of one or more files
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <grp.h>
#include "../cmd.h"
#include "../cmd_err.h"

/*	External functions						*/
/*	==================						*/
struct	group	*getgrnam();

/*	main program							*/
/*	============							*/
main(argc, argv)
register int argc;
register char *argv[];

{
struct	group	*grp;
struct	stat	status;
int	gid, result = 0;
	/*	Initiate prompt string					*/
	PRMPT = *argv;

	--argc;
	++argv;

	/*	Test if enough number of arguments			*/
	if (argc < 2) {
		fprintf(stderr, "usage: %s gid file ...\n", PRMPT);
		exit(4);
	}
	else if (isnumber(*argv))
		gid = atoi(*argv);
	else if ((grp = getgrnam(*argv)) == NULL) {
		fprintf(stderr, BADGID, *argv);
		exit(4);
	}
	else
		gid = grp->gr_gid;

	++argv;

	while (*argv) {
		stat(*argv, &status);
		if (chown(*argv, status.st_uid, gid) == -1) {
			fprintf(stderr, NOCHGRP, *argv);
			perror(*argv);
			result = 1;
		}
		++argv;
	}
	exit(result);
}
