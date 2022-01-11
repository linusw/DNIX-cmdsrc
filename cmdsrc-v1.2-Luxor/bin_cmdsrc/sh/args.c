/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	====================================
*
*	== args.c ==	Read shell arguments
*
*	====================================
*
*	Edit 001	1983-07-15
*/

#include	"defs.h"
extern    int	flags;
int	skipex;

args(argc, argv)
register int argc; char *argv[];
BEGIN
    register int i, fildes[2], child_id;

PRINT("ARGS\n");
	/* Stupid temp fix to handle 'sh prog...' for non x-prev progs */
	skipex = FALSE;
    FOR i = 1; i<argc; ++i DO
	IF *argv[i] == '-' THEN
		WHILE *++argv[i] DO
			IF !setflag(*argv[i]) THEN
				badarg(argv[i], ER_BOP);
			FI
		OD
	ELIF pipe(fildes) THEN
		    perror("shell");
		    exit(-1);
	ELIF (child_id = fork() ) == 0 THEN	/*	child	*/
		WHILE i < argc DO
			IF write(fildes[1], argv[i], strlen(argv[i]))
				!= strlen(argv[i]) THEN
				perror("shell");
			FI
			IF write(fildes[1], " ", 1) != 1 THEN
				perror("shell");
			FI
			i++;
		OD
		exit(0);
	ELIF child_id == -1 THEN
		perror("shell");
		exit(-1);
	ELSE
		clfile(fildes[1]);		/*	parent	*/
		skipex = TRUE;
		return(fildes[0]);
	FI
    OD
    return(0);
END

badarg(a, s)
register char *a, *s;
BEGIN
    errbrk(a);errline(s);exit(-1);
END
