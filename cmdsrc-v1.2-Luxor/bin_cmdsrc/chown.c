/*
 *	chown uid file ...
 *
 *	Copyright (C) DIAB T{by 1983
 *	Written by: Per-Arne Svensson 
 */

/*
 *	change owner of one or more files
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include "../cmd.h"
#include "../cmd_err.h"

/*	External functions						*/
/*	==================						*/
struct	passwd	*getpwnam();

/*	main program							*/
/*	============							*/
main(argc, argv)
register int argc;
register char *argv[];

{
struct	passwd	*pwd;
struct	stat	status;
int	uid, result = 0;

	/*	Initiate prompt string					*/
	PRMPT = *argv;

	--argc;
	++argv;

	/*	Test if enough number of arguments			*/
	if (argc < 2) {
		fprintf(stderr, "usage: %s uid file ...\n", PRMPT);
		exit(4);
	}
	else if (isnumber(*argv))
		uid = atoi(*argv);
	else if ((pwd = getpwnam(*argv)) == NULL) {
		fprintf(stderr, BADUID, *argv);
		exit(4);
	}
	else
		uid = pwd->pw_uid;

	++argv;

	while (*argv) {
		stat(*argv, &status);
		if (chown(*argv, uid, status.st_gid) == -1) {
			fprintf(stderr, NOCHOWN, *argv);
			perror(*argv);
			result = 1;
		}
		++argv;
	}
	exit(result);
}
