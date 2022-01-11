/*
 *	od - octal (hex, decimal, char) dump
 *
 *	Copyright (C) DIAB T{by 1983
 *	Written by: Per-Arne Svensson 
 *	Modified by: Magnus Hedner 1984
 */

/*
 *	dump file
 */

/*
 *	od [ -bcdox ] [ file ] [ [ + ] offset [ . ] [ b ] ]
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include "../cmd.h"
#include "../cmd_err.h"

/*	External functions						*/
/*	==================						*/
int	setbuf();
char	*strcpy(), *strncpy();	/*	Copy a string			*/
char	*strcat(), *strncat();	/*	Concatenate strings		*/
int	strcmp();		/*	Compare strings			*/
int	strlen();		/*	Length of string		*/
long	fseek(), ftell();	/*	Set current stream address	*/

/*	Stream-I/O buffers						*/
/*	==================						*/
char	odbuf[BUFSIZ];		/*	Output stream buffer		*/

/*	Global variables						*/
/*	================						*/

char	line[16];		/*	The current line to dump	*/
char	oldline[16];		/*	The last line that was dumped	*/
int	frstline=TRUE;		/*	Flag indicating first line	*/
int	sameline=FALSE;		/*	Flag indicating that this line	*/
				/*	is the same as the last line	*/
int	bflg=FALSE;		/*	Dump bytes in octal		*/
int	cflg=FALSE;		/*	Dump bytes in ASCII		*/
int	dflg=FALSE;		/*	Dump words in decimal		*/
int	oflg=FALSE;		/*	Dump words in octal		*/
int	xflg=FALSE;		/*	Dump words in hex		*/
int	aflg=FALSE;		/*	Dump all lines			*/
int	nrand=FALSE;		/*	File is not random access	*/
long	address;		/*	Stream position			*/

/*	Main program							*/
/*	============							*/
main(argc, argv)
char *argv[];
int  argc;

{
int	size;			/*	Size of last read line		*/
register int	i;		/*	Loop variable			*/
register unsigned int	n;	/*	Conversion variable		*/
	/*	Initiate prompt string					*/
	PRMPT = *argv;

	/*	Assign stream-I/O buffer to output stream		*/
	setbuf(stdout, odbuf);

	/*	Test if any switches					*/
	--argc;
	++argv;
	while (*argv && **argv == '-') {
		while (*++*argv) switch (**argv) {
		case 'a':	/*	Dump all lines			*/
			aflg = TRUE;
			continue;
		case 'b':	/*	Dump bytes in octal		*/
			bflg = TRUE;
			continue;
		case 'c':	/*	Dump bytes in ASCII		*/
			cflg = TRUE;
			continue;
		case 'd':	/*	Dump words in decimal		*/
			dflg = TRUE;
			continue;
		case 'o':	/*	Dump words in octal		*/
			oflg = TRUE;
			continue;
		case 'x':	/*	Dump words in hex		*/
		case 'h':
			xflg = TRUE;
			continue;
		default:
			fprintf(stderr, BADSW, *argv);
			argc = -1;
		}
		--argc;
		++argv;
	}

	if (argc < 0 || argc >2) {
		fprintf(stderr,"usage: %s [ -bcdox ] [ file ] [ [ + ] offset [ . ] [ b ] ]\n", PRMPT);
		return(-1);
	}

	/*	Set default value if no switches are set		*/
	if (!(bflg | cflg | dflg | oflg | xflg)) {
		oflg = TRUE;
	}

	/*	Test if any filename is given				*/
	if (*argv && **argv != '+') {
		if (freopen(*argv, "r", stdin) == NULL) {
			fprintf(stderr, NOOPEN, *argv);
			exit(1);
		}
		argc--;
		argv++;
	}

	/*	Test if any offset to the file is given			*/
	if (*argv) {
		if (**argv == '+')
			++*argv;
		offset(*argv);
		argc--;
		argv++;
	} else
		offset("0");

	/*	Dump the stream						*/
	for (; size = fread(line, 1, sizeof(line), stdin); address+= size) {

		/*	If last line read is shorter than sizeof(line)	*/
		/*	pad with zeros					*/
		if (size < sizeof(line))
			for (i=size; i<sizeof(line); i++)
				line[i] = 0;

		/*	Skip lines that are equal			*/
		for (i=0; i<sizeof(line); i++)
			if (line[i] != oldline[i])
				break;
		if (i == sizeof(line) && !frstline && !aflg) {
			if (!sameline) {
				fprintf(stdout, "*\n");
				sameline = TRUE;
			}
			continue;
		}

		/*	The old line and the current one are not equal	*/
		for (i=0; i<sizeof(line); i++)
			oldline[i] = line[i];

		/*	Reset flag indicating first line		*/
		frstline = FALSE;
		/*	Reset flag indicating same line as the previous	*/
		sameline = FALSE;

		/*	Print the line					*/
		if(xflg)
			fprintf(stdout, "%07lx", address);
		else
			fprintf(stdout, "%07ld", address);

		if (cflg) {
			fprintf(stdout, "\t");
			for (i=0; i<size; i++) {
				n = (unsigned int)line[i]&0377;

				switch (n) {
					case '\0':
						fprintf(stdout," \\0 ");
						break;

					case '\b':
						fprintf(stdout," \\b ");
						break;

					case '\f':
						fprintf(stdout," \\f ");
						break;

					case '\n':
						fprintf(stdout," \\n ");
						break;

					case '\r':
						fprintf(stdout," \\r ");
						break;

					case '\t':
						fprintf(stdout," \\t ");
						break;

					default:
					if (n > 040 && n < 0177)
						fprintf(stdout, "  %c ", 
							n&0377);
					else
						fprintf(stdout, "%03o ", n);
				}
			}
			fprintf(stdout,"\n");
		}

		if (bflg) {
			fprintf(stdout, "\t");
			for (i=0; i<(size+1>>1); i++) {
				fprintf(stdout, "%03o %03o ", 
					((unsigned int)line[2*i])&0377, 
					((unsigned int)line[2*i+1])&0377);
			}
			fprintf(stdout,"\n");
		}

		if (xflg) {
			fprintf(stdout, "\t");
			for (i=0; i<(size+1>>1); i++) {
				n = (unsigned int)line[2*i]&0377;
				n <<= 8;
				n |= (unsigned int)line[2*i+1]&0377;
				fprintf(stdout, "%04x    ", n);
			}
			fprintf(stdout,"\n");
		}

		if (dflg) {
			fprintf(stdout, "\t");
			for (i=0; i<(size+1>>1); i++) {
				n = (unsigned int)line[2*i]&0377;
				n <<= 8;
				n |= (unsigned int)line[2*i+1]&0377;
				fprintf(stdout, "%05u   ", n);
			}
			fprintf(stdout,"\n");
		}

		if (oflg) {
			fprintf(stdout, "\t");
			for (i=0; i<(size+1>>1); i++) {
				n = (unsigned int)line[2*i]&0377;
				n <<= 8;
				n |= (unsigned int)line[2*i+1]&0377;
				fprintf(stdout, "%06o  ", n);
			}
			fprintf(stdout,"\n");
		}
	}
	return(0);
}



/*	Function for positioning the stream stdin - offset()		*/
offset(string)
char *string;
{
long	spos=0;			/*	Search position			*/
register char	*pos;		/*	Character address		*/
register int	chr;		/*	Character			*/
register int	base=8;		/*	Number base			*/
	pos = string;

	/*	Decode the base of the conversion			*/
	if (*pos == 'x') {
		base = 16;
		pos++;
	} else if (*pos == '0' && pos[1] == 'x') {
		base = 16;
		pos+=2;
	} else if (*pos == '0') {
		base = 8;
		pos++;
	}
	string = pos;

	while (chr = *pos++)
		if (chr == '.')
			base = 10;
	pos = string;
	address = 0L;

	/*	Decode the address					*/
	while (chr = *pos++) {
		if (isdigit(chr))
			address = address * (long)base + (long)(chr - '0');
		else if (chr >= 'a' && chr <= 'f' && base == 16)
			address = address * (long)base + (long)(chr + 10 - 'a');
		else
			break;
	}

	/*	Skip the eventual decimal point				*/
	if (chr == '.')
		chr = *pos;

	/*	Test if the address is in 512-byte blocks		*/
	if (chr)
		if (chr == 'b' || chr == 'B')
			address *= (long)512;

	/*	Position input stream					*/
	if(fseek(stdin, address, 0))
		for(nrand = TRUE; spos++ < address && getchar() != EOF; );
	else
		address = ftell(stdin);	/*	Just for sure...	*/
}
