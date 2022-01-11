/*
 *	ln - link files
 *
 *	Copyright (C) DIAB T{by 1983
 *	Written by: Per-Arne Svensson 
 */

/*
 *	ln [ -f ] dest [ source ]
 *
 *	Links source file or, if omitted, a file in the current directory
 *	with the same directory name as dest , to the dest file.
 *	If the flag -f is given, a link can be made to a directory.
 *
 */



#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../cmd.h"	/*	Command include-file			*/
#include "../cmd_err.h"	/*	Command-error include-file		*/

			/*	External declarations			*/
			/*	=====================			*/

char	*pname();	/*	Function returning pathname		*/
char	*dname();	/*	Function returning directory-name	*/
struct	entry *makentry();	/*	Make one tree entry		*/
struct	entry *freetree();	/*	Free memory occupied by tree	*/
struct	stat  *getstat();	/*	Make one status block entry	*/

/*	Global variables						*/
/*	================						*/

int	fflg=FALSE;		/*	Force to link directory		*/

main(argc, argv)
char	*argv[];		/*	Argument pointer		*/
int	argc;			/*	Argument count			*/
{
struct	entry	*dst_entry;	/*	Destination file entry		*/
struct	entry	*src_entry;	/*	Source file entry		*/
struct	entry	*tmp_entry;	/*	Temporary file entry		*/

	/*	Initiate prompt string					*/
	PRMPT = *argv;

	/*	Test if any switches					*/
	--argc;
	++argv;
	while (*argv && **argv == '-') {
		while (*++*argv) switch (**argv) {
		case 'f':		/*	Force to link directory	*/
			fflg = TRUE;
			continue;
		default:
			fprintf(stderr, BADSW, *argv);
			argc = -1;
		}
		--argc;
		++argv;
	}

	/*	Check if number of arguments is too small		*/
	/*	=========================================		*/

	if	(argc > 2 || argc < 1)	goto usage;

	/*	Make a dest entry					*/
	/*	===================					*/

	dst_entry = makentry(pname(*argv), dname(*argv));

	--argc;
	++argv;
	
	if (getstat(dst_entry) == NULL) {
		fprintf(stderr, NOACC, dst_entry->e_fname);
		exit(1);
	}

	if (fflg == 0 && isdir(*(dst_entry->e_stat))) {
		fprintf(stderr, FISDIR, dst_entry->e_fname);
		exit(1);
	}

	/*	Make a source entry					*/
	/*	===================					*/

	if (*argv) {
		src_entry = makentry(pname(*argv), dname(*argv));
		/*	If source file is a directory, 			*/
		/*	add the dirname of dest				*/
		if (getstat(src_entry) != NULL &&
			isdir(*(src_entry->e_stat))) {
			tmp_entry = src_entry;
			src_entry = makentry(tmp_entry->e_fname, 
					dname(dst_entry->e_fname));
			freetree(tmp_entry);
		}
	} else
		src_entry = makentry("", dname(dst_entry->e_fname));

	/*	Link the files						*/
	/*	==============						*/

	if (link(dst_entry->e_fname, src_entry->e_fname) != 0) {
		fprintf(stderr, NOLINK, dst_entry->e_fname, 
					src_entry->e_fname);
		perror(PRMPT);
		exit(1);
	}
	exit(0);

usage:
	fprintf(stderr, "usage: %s [ -f ] dest [ source ]\n", PRMPT);
	exit(1);
}
