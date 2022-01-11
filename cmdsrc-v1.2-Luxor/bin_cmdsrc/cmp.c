/*
 * cmp - command
 *
 *	Copyright (C) DIAB T{by 1984
 *	Written by: Henrik Burton
 */

/*
 *	compare files
 *
 *	cmp [ -l ] [ -s ] file1 file2 [ pos1 [ pos2 ]]
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../cmd.h"
#include "../cmd_err.h"

char posit();
long otoi();

char	usage[] = "usage:  %s [ -l ][ -s ] file1 file2 [ pos1 [ pos2 ]]\n";

/*
 *	Main program
 *	============
 */

main(argc, argv)
int	argc;
char	*argv[];
{
	register int	ch1;
	register int	ch2;
	register long	pos;
	int	lflg = FALSE;
	int	sflg = FALSE;
	int	diff = FALSE;
	long	linenr = 1L;
	char	*arg;
	FILE	*file1;
	FILE	*file2;

	pos = 0L;
	PRMPT = *argv++;	/* initiate promt string */

	while (--argc > 0) {
		arg = *argv++;
		if (arg[0] == '-' && arg[1] != '\0')
			switch (arg[1]) {
				case 'l' :
					lflg = TRUE;
					break;
				case 's' :
					sflg = TRUE;
					break;
				default :
					fprintf(stderr, usage, PRMPT);
					exit(2);
			}
		else
			break;
	}

	if (argc < 2) {
		fprintf(stderr, usage, PRMPT);
		exit(2);
	}
	else
		if (arg[0] == '-' && arg[1] == '\0')
			file1 = stdin;
		else
			if ((file1 = fopen(arg, "r")) == NULL) {
				fprintf(stderr, NOACC, arg);
				exit(2);
			}

	arg = *argv++;

	if ((file2 = fopen(arg, "r")) == NULL) {
		fprintf(stderr, NOACC, arg);
		exit(2);
	}

	if (argc > 2)
		posit(file1, otoi(*argv));
	++argv;

	if (argc > 3)
		posit(file2, otoi(*argv));

	while (TRUE) {
		++pos;
		if ((ch1 = getc(file1)) == (ch2 = getc(file2)))
			if (ch1 == '\n')
				++linenr;
		else
				if (ch1 == EOF)
					exit(diff ? 1 : 0);
				else
					continue;
		else {
			if (!lflg && sflg)
				exit(1);
			if (ch1 == EOF || ch2 == EOF) {
				fprintf(stderr,"%s:  EOF on %s\n", PRMPT,
					ch1 == EOF ? *(argv-3) : *(argv-2));
				exit(1);
			}
			if (lflg == sflg) {
				printf("%s %s differ: char %ld, line %ld\n",
					*(argv-3), *(argv-2), pos, linenr);
				exit(1);
			}
			diff = TRUE;
			printf("%6ld %3o %3o\n", pos, ch1, ch2);
		}
	}
}

/*
 *	procedure posit; posit file fp to positon p
 *	===========================================
 */

char posit(fp, p)
FILE	*fp;
long	p;
{
	register char	c;

	while (p--)
		if ((c = getc(fp)) == EOF)
			break;
	return(c);
}

/*
 *	procedure otoi; convert a octal or decimal string to long
 *	=========================================================
 */

long otoi(s)
register char *s;
{
	register char c;
	register long radix;
	register long value;

	value = 0L;
	radix = 10L;		/* default decimal */

	if (s[0]  == '0')
		radix = 8L;	/* select octal */
	while (c = *s++ - '0', c >= 0 && c < (int) radix)
		value = value * radix + c;
	return(value);
}
