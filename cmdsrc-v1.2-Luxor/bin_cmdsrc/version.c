/*
 *	version [-ver_nr] file ...
 *
 *	Copyright (C) DIAB T{by 1983
 *	Written by: Per-Arne Svensson 
 */

/*
 *	change version of one or more files
 */
/*	Per-Arne Svensson DIAB T{by			*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <a.out.h>
#include <ctype.h>
#include "../cmd.h"
#include "../cmd_err.h"

/*	main program							*/
/*	============							*/
main(argc, argv)
register int argc;
register char *argv[];

{
struct	xexec xhead;		/*	x.out header-record		*/
struct	stat	status;		/*	FIle status block		*/
int	ver_nr = -1, filedsc;

	/*	Initiate prompt string					*/
	PRMPT = *argv;

	if (getuid() != 0) {	/*	No permission			*/
		fprintf(stderr, NOPERM);
		exit(1);
	}

	--argc;
	++argv;

	setuid(getuid());

	/*	Test if enough number of arguments			*/
	if (argc < 1 ) {
		fprintf(stderr, "usage: %s [-ver_nr] file ...\n", PRMPT);
		exit(4);
	}
	if (**argv == '-' && isdigit(*++*argv))
		ver_nr = atoi(*argv++);

	while (*argv) {
		if ((access(*argv,2)) == 0 && ver_nr != -1) {
			stat(*argv, &status);
			if ((status.st_mode & 0111) == 0 ||
				ver_nr > 3 || ver_nr < -1)
				fprintf(stderr, NOCHVER, *argv);

			/*	Open file and change version		*/
			else if ((filedsc = open(*argv, 2)) != -1) {

				/*	Read exec header record		*/
				if (read(filedsc, &xhead, 
					sizeof(xhead)) == sizeof(xhead)) {
					xhead.x_renv &= ~0xC000;
					xhead.x_renv |= ver_nr << 14;

					/* Position to beg of file	*/
					lseek(filedsc, 0L, 0);

					/* Write back record		*/
					if (write(filedsc, &xhead, 
					sizeof(xhead)) != sizeof(xhead))
						fprintf(stderr, WRERR, *argv);
				} else
					fprintf(stderr, RDERR, *argv);
				close(filedsc);
			} else
				fprintf(stderr, NOOPEN, *argv);
		}
		if (access(*argv, 4) == 0) {

			/*	Open file and read version		*/
			if ((filedsc = open(*argv, 0)) != -1) {

				/*	Read exec header record		*/
				if (read(filedsc, &xhead, 
				sizeof(xhead)) == sizeof(xhead))

					/* Print version number bits	*/
					fprintf(stdout, "-%d %s\n", xhead.x_renv >> 14, 
						*argv);
				else
					fprintf(stderr, RDERR, *argv);
				close(filedsc);
			} else
				fprintf(stderr, NOOPEN, *argv);
		} else
			fprintf(stderr, NOACC, *argv);
		++argv;
	}
	exit(0);
}
