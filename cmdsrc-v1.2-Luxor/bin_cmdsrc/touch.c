/*
 * touch file ...
 *
 *	Copyright (C) DIAB T{by 1983
 *	Written by: Per-Arne Svensson 
 */

/*
 *	change date of modification on one or more files
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../cmd.h"
#include "../cmd_err.h"

/*	main program							*/
/*	============							*/
main(argc, argv)
register int argc;
register char *argv[];
{
int	cflg	= TRUE;	/*	Force file to be created	*/
int	result	= 0;		/*	Return status			*/

	/*	Initiate prompt string					*/
	PRMPT = *argv;

	/*	Test if enough number of arguments			*/
	while (*++argv) {
		if (**argv == '-' && argv[0][1] == 'c')
			cflg = FALSE;
		 else
			switch (touch(cflg, *argv)) {
			case 1:
				fprintf(stderr, NOCREA, *argv);
				result = 1;
				break;
			case 2:
				fprintf(stderr, NOOPEN, *argv);
				result = 1;
				break;
			case 3:
				fprintf(stderr, RDERR, *argv);
				result = 1;
				break;
			case 4:
				fprintf(stderr, WRERR, *argv);
				result = 1;
				break;
			case 5:
				fprintf(stderr, NOACC, *argv);
				result = 1;
				break;

			}
	}
	exit(result);
}

/*	Function for changing date of modification on file - touch	*/
/*	==========================================================	*/

/*	Return status:
			0:	OK.
			1:	Cannot create file.
			2:	Cannot open file.
			3:	Cannot read file.
			4:	Cannot write file.
			5:	Cannot access file.
 */

touch(create, filename)
int	create;			/*	Force creation of file		*/
char	*filename;
{
struct	stat status;		/*	File status			*/
char	chr[1];			/*	Character read and written	*/
register int filedsc;		/*	File descriptor			*/

	/*	Test if file should be created				*/
	if (stat(filename, &status) != 0) {
		if (create) {
			if ((filedsc = creat(filename, 0666)) < 0) {
				close(filedsc);
				return(1);	/*	No create	*/
			} else
			return(0);
		} else
			return(5);		/*	No access	*/
	}

	if (status.st_size == 0) {
		if ((filedsc = creat(filename, 0666)) < 0) {
			close(filedsc);
			return(1);		/*	No create	*/
		} else
		return(0);
	}

	/*	Try to open the file, read one char, and write it back	*/
	if ((filedsc = open(filename, 2)) == -1)
		return(2);			/*	No open		*/

	if (read(filedsc, chr, 1) != 1) {
		close(filedsc);
		return(3);			/*	No read		*/
	}

	lseek(filedsc, 0L, 0);

	if (write(filedsc, chr, 1) != 1) {
		close(filedsc);
		return(4);			/*	No write	*/
	}

	close(filedsc);
	return(0);
}
