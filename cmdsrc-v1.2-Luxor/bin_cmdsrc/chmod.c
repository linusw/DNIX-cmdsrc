/*
 * chmod [ugoa][+-=][rwxstugo],... file ...
 *
 *	Copyright (C) DIAB T{by 1983
 *	Written by: Per-Arne Svensson 
 */

/*
 *	change mode of one or more files
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../cmd.h"
#include "../cmd_err.h"

/*	External functions						*/
/*	==================						*/
int	chmod();
extern	char	*filemode();	/*	Get file mode in ASCII		*/

/*	main program							*/
/*	============							*/
main(argc, argv)
register int argc;
register char *argv[];

{
register char	*mode;
struct	stat	status;
int	result = 0;
	/*	Initiate prompt string					*/
	PRMPT = *argv;

	--argc;
	++argv;

/*	Test if enough number of arguments			*/
	if (argc < 2) {
		fprintf(stderr, 
		"usage: %s [ugoa][+-=][rwxstugo],...  file ...\n", PRMPT);
		exit(4);
	}

	mode = *argv++;
	--argc;

	while (*argv) {
		stat(*argv, &status);
		if (chmod(*argv, getmode(mode, status.st_mode)) < 0) {
			fprintf(stderr, NOCHMOD, *argv);
			perror(*argv);
			result = 1;
		}
		++argv;
	}
	exit(result);
}

/*	Function converting mode to an integer - getmode		*/
/*	================================================		*/
getmode(string, oldmode)
char	*string;
unsigned short oldmode;
{
int op_type;
register unsigned short newmode = 0, andmsk, ormsk, chr;

	/*	Check if the string is in octal format			*/
	while ((chr = *string) <= '7' && chr >= '0') {
		newmode = (newmode << 3) + chr - '0';
		string++;
	}
	if (!*string)
		return(newmode);
	else

	newmode = oldmode;
	do {
		andmsk = ormsk = 0;
		if (who(string))
			while (who(string))
				switch(*string++) {
				case 'u':
					andmsk |= USER;
					break;
				case 'g':
					andmsk |= GROUP;
					break;
				case 'o':
					andmsk |= OTHERS;
					break;
				case 'a':
					andmsk |= ALL;
					break;
				}	
		else
			if (andmsk == 0)
				andmsk = ALL & ~umask(0);
		if (!(op_type = op(string++)))
			goto error;
		if (who(string)) {
			switch(*string++) {
			case 'u':
				ormsk = (oldmode & USER) >> 6;
				break;
			case 'g':
				ormsk = (oldmode & GROUP) >> 3;
				break;
			case 'o':
				ormsk = oldmode & OTHERS;
				break;
			default:
				goto error;
			}
			ormsk &= READ|WRITE|EXEC;
			ormsk |= (ormsk << 6) | (ormsk << 3);
		}
		else if (perm(string))
			while (perm(string))
				switch(*string++) {
				case 'r':
					ormsk |= READ;
					continue;
				case 'w':
					ormsk |= WRITE;
					continue;
				case 'x':
					ormsk |= EXEC;
					continue;
				case 's':
					ormsk |= SETID;
					continue;
				case 't':
					ormsk |= STICKY;
					continue;
				}
		else if (*string)
			goto error;
		switch (op_type) {
		case '+':
			newmode |= ormsk & andmsk;
			break;
		case '-':
			newmode &= ~(ormsk & andmsk);
			break;
		case '=':
			newmode &= ~andmsk;
			newmode |= ormsk & andmsk;
			break;
		}
#ifdef	DEBUG
	fprintf(stderr, "newmode: %0o andmsk: %0o ormsk: %0o\n", newmode, andmsk, ormsk);
#endif
	} while (*string++ == ',');
	if (*--string) {
error:
	fprintf(stderr, BADMODE, string);
	exit(255);
	}
	else
		return(newmode);

}

/*	Function for determining whether string contains an op		*/
/*	======================================================		*/
op(string)
register char *string;
{
#ifdef	DEBUG
	fprintf(stderr, "op(%s)\n", string);
#endif
	switch(*string) {
	case '+':
	case '-':
	case '=':
		return(*string++);
	default:
		return(0);
	}
}

/*	Function for determining whether string contains who		*/
/*	====================================================		*/
who(string)
register char *string;
{
#ifdef	DEBUG
	fprintf(stderr, "who(%s)\n", string);
#endif
	switch(*string) {
	case 'u':
	case 'g':
	case 'o':
	case 'a':
		return(*string++);
	default:
		return(0);
	}
}

/*	Function for determining whether string contains permission	*/
/*	===========================================================	*/
perm(string)
register char *string;
{
#ifdef	DEBUG
	fprintf(stderr, "perm(%s)\n", string);
#endif
	switch(*string) {
	case 'r':
	case 'w':
	case 'x':
	case 's':
	case 't':
		return(*string++);
	default:
		return(0);
	}
}

