/*
 *	print text to standard output or standard error output
 *	SYSTEM 3 compatible
 *
 *	Copyright (C) DIAB T{by 1984
 *	Written by: Magnus Hedner
 */

/*
 *	echo [-n] [-e] [--] [arg] ...
 */

#include <stdio.h>
#include "../cmd.h"
#include "../cmd_err.h"
char	buf[BUFSIZ];
FILE	*outfile;

/*	main program							*/
/*	============							*/
main(argc, argv)
int	argc;
register char	*argv[];
{
	char c;
	int	nflg = TRUE, eflg = FALSE, flgflg = TRUE, i;

	PRMPT = *argv;
	++argv;

	/*	Test arguments						*/
	while(*argv && **argv == '-') {
		while(*argv && *++*argv) switch(**argv) {
			case 'n':/*	Suppress ending newline		*/
				nflg = flgflg = FALSE;
				break;
			case 'e':/*	Direct output to stderr		*/
				eflg = TRUE;
				flgflg = FALSE;
				break;
			case '-':/*	Double minus to end args	*/
				argv++;
				goto ARGEND;
			default:/*	Treat as argument		*/
				if(flgflg)
					(*argv)--;
				goto ARGEND;
			}
		argv++;
		flgflg = TRUE;
	}

ARGEND:

	outfile = (eflg ? stderr : stdout);

	setbuf(outfile, buf);

	while(*argv && **argv) {
		if(**argv != '\\')
			putc(**argv, outfile);
		else
			switch(*++*argv) {
				case 'b':/*	backspace		*/
					putc('\b', outfile);
					break;
				case 'c':/*	same as -n option	*/
					nflg = FALSE;
					break;
				case 'f':/*	form feed		*/
					putc('\f', outfile);
					break;
				case 'n':/*	newline			*/
					putc('\n', outfile);
					break;
				case 'r':/*	Carriage return		*/
					putc('\r', outfile);
					break;
				case 't':/*	tabulate		*/
					putc('\t', outfile);
					break;
				case '\\':/*	backslash		*/
					putc('\\', outfile);
					break;
				case '0':/*	Octal char 0xxx		*/
					i = 0;
					c = '\0';
					while(*++*argv <= '7' && **argv >= '0'
					  && i++ < 3) {
						c <<= 3;
						c += (**argv - '0');
					}
					--*argv;
					putc(c, outfile);
					break;
				default:/*	What is this???		*/
					putc(**argv, outfile);
					break;
				}
		++*argv;
		if(**argv == '\0') {
			argv++;
			if(*argv)
				putc(' ', outfile);
		}
	}

	if(nflg)
		putc('\n', outfile);
}
