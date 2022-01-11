/*
 *	od-1 - inverse octal (hex, decimal, char, binary) dump
 *
 *	Copyright (C) DIAB T{by 1983
 *	Written by: Per-Arne Svensson 
 */

/*
 *	od-1 [ -bcdox ] [ file ]
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
long	fseek();		/*	Set current stream address	*/
long	lseek();		/*	Set current file address	*/

/*	Stream-I/O buffers						*/
/*	==================						*/
char	odbuf[BUFSIZ];		/*	Output stream buffer		*/

/*	Global variables						*/
/*	================						*/

char	line[16];		/*	The current line to dump	*/
int	bflg=FALSE;		/*	Dump bytes in octal		*/
int	cflg=FALSE;		/*	Dump bytes in ASCII		*/
int	dflg=FALSE;		/*	Dump words in decimal		*/
int	oflg=FALSE;		/*	Dump words in octal		*/
int	xflg=FALSE;		/*	Dump words in hex		*/

/*	Main program							*/
/*	============							*/
main(argc, argv)
char *argv[];
int  argc;

{
register int	end_file;	/*	End of input stream		*/
register int	found;		/*	Match flag			*/
register int	i;		/*	Loop variable			*/
off_t		address;	/*	Stream address			*/
unsigned int	n;		/*	Conversion variable		*/
int		fdes = 0;	/*	File descriptor			*/
	/*	Initiate prompt string					*/
	PRMPT = *argv;

	/*	Assign stream-I/O buffer to output stream		*/
	setbuf(stdout, odbuf);

	/*	Test if any switches					*/
	--argc;
	++argv;
	while (*argv && **argv == '-') {
		while (*++*argv) switch (**argv) {
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

	if (argc < 0 || argc >1) {
		fprintf(stderr,"usage: %s [ -bcdox ] [ file ]\n", PRMPT);
		return(-1);
	}

	/*	Test if any filename is given				*/
	if (*argv) {
		if ((fdes = open(*argv, 2)) <= 0) {
			fprintf(stderr, NOOPEN, *argv);
			exit(1);
		}
	}

	/*	Set default value if no switches are set		*/
	if (!(bflg | cflg | dflg | oflg | xflg)) {
		oflg = TRUE;
	}

	/*	Read the stream						*/
	while (!(end_file = (found=fscanf(stdin, xflg?"%lx":"%ld", &address))==EOF)) {

		/*	Read the line					*/
		;
		if (!found) badconv();

		if (cflg) {
			register char	chr[5];
			for (i=0; i<sizeof(line) &&
				!(end_file=((found=fscanf(stdin,"%4s",chr))
					==EOF)); i++) {
				if ((n = strlen(chr))==1)
					n = chr[0];
				else if (n == 2 && chr[0] == '\\')
					switch (chr[1]) {
						case '0':
							n = 0;
							found=TRUE;
							break;
						case 'b':
							n = 8;
							found=TRUE;
							break;
						case 't':
							n = 9;
							found=TRUE;
							break;
						case 'f':
							n = 12;
							found=TRUE;
							break;
						case 'n':
							n = 10;
							found=TRUE;
							break;
						case 'r':
							n = 13;
							found=TRUE;
							break;
						default:
							found=FALSE;
					}
				else if (n == 3 && (found=sscanf(chr,"%3o ",&n)));

				if (!found) badconv();
				if (end_file) {i--;break;}
				line[i] = (char)n;
			}
		}

		if (bflg) {
			for (i=0; i<sizeof(line) &&
				!(end_file = ((found=fscanf(stdin, "%3o ", &n))==EOF));
				i++) {

				if (!found) badconv();
				line[i] = (char)n;
			}
		}

		if (xflg) {
			for (i=0; i<sizeof(line) && 
				!(end_file =((found=fscanf(stdin, "%4x ", &n))==EOF));
				i++,i++) {
				
				if (!found) badconv();
				line[i] = (n >> 8)&0377;
				line[i+1] = n&0377;
			}
		}

		if (dflg) {
			for (i=0; i<sizeof(line) &&
				!(end_file = ((found=fscanf(stdin, "%5u ", &n))==EOF));
				i++,i++) {

				if (!found) badconv();
				line[i] = (n >> 8)&0377;
				line[i+1] = n&0377;
			}
		}

		if (oflg) {
			for (i=0; i<sizeof(line) &&
				!(end_file = ((found=fscanf(stdin, "%6o", &n))==EOF));
				i++,i++) {
				
				if (!found) badconv();
				line[i] = (n >> 8)&0377;
				line[i+1] = n&0377;
			}
		}

		if (!*argv && !end_file) {
			fseek(stdout, address, 0L);
			if (fwrite(line, i, 1, stdout) != 1) {
				fprintf(stderr, WRERR, "stdout");
				exit(-1);
			}
		} else if (!end_file) {
			lseek(fdes, address, 0L);
			if (write(fdes, line, i) != i) {
				fprintf(stderr, WRERR, *argv);
				exit(-1);


			}
		}
	}
	if (*argv) close(fdes);
	return(0);
}
badconv()
{
	fprintf(stderr, BADCONV);
	exit(-1);
}
