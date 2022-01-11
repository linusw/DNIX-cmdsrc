/*
 * cat - command
 *
 *	Copyright (C) DIAB T{by 1983
 *	Written by: Per-Arne Svensson 
 */

/*
 *	concatenate files
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../cmd.h"
#include "../cmd_err.h"

char	catbuf[BUFSIZ];		/*	Cat stream/io buffer		*/

/*	Main Program							*/
/*	============							*/
main(argc, argv)
register char	*argv[];
register int	argc;

{
struct	stat srcstat, dststat;	/*	Status blocks			*/
				/*	FLAGS				*/
int	uflg	= FALSE;	/*	Unbuffered output		*/
int	normal	= FALSE;	/*	Normal output file		*/
register FILE *infile;		/* Input file pointer */

	/*	Initiate prompt string					*/
	PRMPT = *argv;

	++argv;
	--argc;

	while (*argv && **argv == '-' && (*argv)[1]) {
		switch((*argv)[1]){
		case 'u':	/*	Don't buffer in 512-byte blocks	*/
			uflg = TRUE;
			argv++;
			argc--;
			break;
		default:

			fprintf(stderr, BADSW, *argv);
			return(1);
		}
	}

	if (uflg == TRUE)	/*	Assign buffer to output stream	*/
		setbuf(stdout, (char *)NULL);
	else
		setbuf(stdout, catbuf);

	fstat(fileno(stdout), &dststat);	/*	Get file status	*/
	if (dststat.st_mode & S_IFMT != S_IFCHR &&
		dststat.st_mode & S_IFMT != S_IFBLK)
		normal = TRUE;

	if (argc == 0 )
		cat(stdin, stdout);
	else
	while (argc > 0) {

		if((*argv)[0] == '-' && (*argv)[1] == '\0'){
			infile = stdin;
		} else {
			if((infile = fopen(*argv,"r")) == NULL){
				fprintf(stderr, NOOPEN, *argv);
				argv++;
				argc--;
				continue;
			}
		}
		if (fstat(fileno(infile), &srcstat) == -1){
			fprintf(stderr, NOACC, *argv);
		}
		else if (normal == TRUE && 
			srcstat.st_dev == dststat.st_dev &&
			srcstat.st_ino == dststat.st_ino){
			fprintf(stderr, INISOUT, *argv);
		} else {
			cat(infile, stdout);
		}
		if(infile != stdin){
			fclose(infile);
		}
		++argv;
		--argc;
	}
	return(0);
}

/*	Function for concatening two streams - cat			*/
/*	==========================================			*/
cat(instr, outstr)
register FILE *instr, *outstr;
{
register chr;			/*	Cat-loop invariant		*/
	while ((chr = getc(instr)) != EOF)
		putchar(chr);
	return;
}
