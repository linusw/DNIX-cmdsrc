/*
 *	unlink (interactive command) 
 *
 *	Copyright (C) DIAB T{by 1983
 *	Written by: Per-Arne Svensson 
 */

/*
 *	remove one or more files interactively from current directory
 */
/*	Per-Arne Svensson DIAB T{by			*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include "../cmd.h"
#include "../cmd_err.h"

/*	External functions						*/
/*	==================						*/
char	*pname();		/*	Get path-name of file		*/
char	*dname();		/*	Get directory-name of file	*/
char	*strcpy();		/*	Copy a string			*/
char	*strcat();		/*	Concatenate strings		*/
int	strcmp();		/*	Compare strings			*/
struct	entry *makentry();	/*	Make one tree entry		*/
struct	entry *freetree();	/*	Free memory occupied by tree	*/
struct	stat  *getstat();	/*	Make one status block entry	*/
char	*filemode();		/*	Get file-permissions		*/
int	rmvdir();		/*	Remove directory entry		*/

remove();			/*	Remove one file (recursive)	*/
fileinfo();			/*	Detailed info about file	*/

/*	Main program							*/
/*	============							*/
main(argc, argv)
char *argv[];
int  argc;

{
register int result = 0;	/*	Return variable			*/

	/*	Initiate prompt string					*/
	PRMPT = *argv;

	++argv;
	--argc;

	rmv_int();
}

/*	Function for interactive removal of files from current directory*/
/*	================================================================*/
rmv_int()
{
int dirfile;				/*	File descriptor		*/
struct direct	dir_entry;		/*	Directory entry		*/

		/*	Open the directory				*/
		if ((dirfile = open(DOT, 0)) < 0) {
			fprintf(stderr, NOOPEN, DOT);
			return(1);
		}

		/*	Loop and remove files				*/
		while (sizeof(dir_entry) ==
			read(dirfile, &dir_entry, sizeof(dir_entry))) {

			/*	Test if the file should be removed	*/
			if (dir_entry.d_ino == 0 ||
				!strcmp(dir_entry.d_name, DOT) ||
				!strcmp(dir_entry.d_name, DOTDOT))
				continue;
				if(ask(TRUE, RMFILE, dir_entry.d_name) 
					== FALSE) continue;
				if (unlink(dir_entry.d_name) != 0)
					fprintf(stderr, NOUNLN, 
						dir_entry.d_name);

		}
		close(dirfile);
		return(0);
}


