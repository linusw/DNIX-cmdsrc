/*
 *	rm [-frvi] file ...
 *
 *	Copyright (C) DIAB T{by 1983
 *	Written by: Per-Arne Svensson 
 */

/*
 *	remove one or more files
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

/*	Global variables						*/
/*	================						*/

int	fflg	=	FALSE;	/*	Force files to be deleted	*/
int	iflg	=	FALSE;	/*	Interactive mode		*/
int	rflg	=	FALSE;	/*	Recursive remove of directories	*/
int	vflg	=	FALSE;	/*	Verbouse option			*/
short	uid;			/*	User ID				*/

remove();			/*	Remove one file (recursive)	*/
fileinfo();			/*	Detailed info about file	*/

/*	Main program							*/
/*	============							*/
main(argc, argv)
char *argv[];
int  argc;

{
register struct entry	*root = NULL;	/*	Root of tree to remove	*/
register int result = 0;	/*	Return variable			*/

	/*	Initiate prompt string					*/
	PRMPT = *argv;

	++argv;
	--argc;

	/*	Get user ID						*/
	uid = getuid();

	while (*argv && **argv == '-') {
		while(*++*argv) switch (**argv) {
		case 'f':	/*	Force files to be removed	*/
			fflg = TRUE;
			continue;
		case 'i':	/*	Ask the user before removing file*/
			iflg = TRUE;
			continue;
		case 'r':	/*	Recursive remove of files	*/
			rflg = TRUE;
			continue;
		case 'v':	/*	Detailed info about every file	*/
			vflg = TRUE;
			continue;
		default:
			fprintf(stderr, BADSW, *argv);
			argc = 0;
		}
		++argv;
		--argc;
	}

	if (argc <= 0) {
		fprintf(stderr,"usage: %s [-frvi] file ...\n", PRMPT);
		return(-1);
	}
	while (*argv) {
		root = makentry(pname(*argv), dname(*argv));
		result |= remove(root, fflg, iflg, rflg);
		freetree(root);
		argv++;
	}
	return(result);
}

/*	Function for recursive removal of files				*/
/*	=======================================		=		*/
remove(ptr, force, interact, recurse)
struct	entry	*ptr;		/*	File entry pointer		*/
int	force, interact, recurse;
{
int dirfile;				/*	File descriptor		*/
struct direct	dir_entry;		/*	Directory entry		*/
register struct entry	*tree = NULL;	/*	Root of tree to remove	*/
register int result = 0;		/*	Return variable		*/

	if (getstat(ptr) == NULL) {
		/*	File cannot be accessed		*/
		if(!force){
			fprintf(stderr, NOACC, ptr->e_fname);
			return(1);
		} else {
			return(0);
		}
	}

	switch (ptr->e_stat->st_mode & S_IFMT) {
	case S_IFDIR:

		if (!recurse) {
			/*	File is a directory			*/
			fprintf(stderr, FISDIR, ptr->e_fname);
			return(1);
		}
		
		if (ask(interact, RMVDIR, ptr->e_fname) == FALSE)
			return(1);

		if (access(ptr->e_fname, 2) < 0) {
			/*	File not accessible	*/
			if(!force){
				fprintf(stderr, NWRACC, ptr->e_fname);
			}
			return(1);
		}
			
		/*	Open the directory				*/
		if ((dirfile = open(ptr->e_fname, 0)) < 0) {
			fprintf(stderr, NOOPEN, ptr->e_fname);
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

			/*	Make a new filename			*/

			if ((tree = makentry(ptr->e_fname, 
				dir_entry.d_name)) == NULL)
				return(1);
			/*	Recurse				*/
			result += remove(tree, force, interact, recurse);
			freetree(tree);
		}
		close(dirfile);
		/*	Remove directory				*/
		if (result == 0)
			result = rmvdir(ptr);
		break;

	default:
		if(ask(interact, RMFILE, ptr->e_fname) == FALSE)
			return(0);

		/*	Check if the file is accessible			*/
		if (access(ptr->e_fname, 02) != 0) {
			if (!force) {
				fprintf(stderr, MODE, ptr->e_fname, 
					filemode(ptr->e_stat));
				if(ask(!force, OVRMSG, ptr->e_fname) == FALSE)
				return(0);
			}
		}

		if (unlink(ptr->e_fname) != 0) {
			if(!force || interact){
				fprintf(stderr, NOUNLN, ptr->e_fname);
				return(1);
			} else {
				return(0);
			}
		}
	}

	/*	Test if verbous flag			*/
	if (vflg && result == 0)
		printf("%s\n", ptr->e_fname);
	return(result);
}


