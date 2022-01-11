/*
 *	cp - copy files
 *
 *	Copyright (C) DIAB T{by 1983
 *	Written by: Per-Arne Svensson 
 */

/*
 *	cp f1 f2; or cp f1 ... fn d2;
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

			/*	Copy-buffer				*/
			/*	===========				*/

char	cpbuf[BUFSIZ];

main(argc, argv)
char	*argv[];		/*	Argument pointer		*/
int	argc;			/*	Argument count			*/
{
struct	entry	*dst_entry;	/*	Destination file entry		*/
struct	entry	*src_entry;	/*	Source file entry		*/
register i, r;			/*	Loop variables			*/

	/*	Initiate prompt string					*/
	PRMPT = *argv;

	/*	Set file-protection mask			*/
	/*	========================			*/
	umask(0);

	/*	Check if number of arguments is too small	*/
	/*	=========================================	*/
	if	(argc < 3 )	goto usage;

	/*	Make a target entry				*/
	/*	===================				*/
	dst_entry = makentry(pname(argv[argc-1]), dname(argv[argc-1]));
	
	/*	Is it a directory-copy				*/
	/*	======================				*/
	if	(argc > 3) {

		/*	Check target file-status		*/
		/*	========================		*/
		if	(getstat(dst_entry) == NULL) {
			fprintf(stderr, NOACC, dst_entry->e_fname);
			goto usage;
		}

		/*	Target-file must be a directory		*/
		/*	===============================		*/
		if	(!isdir(*(dst_entry->e_stat))) goto usage;
	}

	/*	Loop and move the files				*/
	/*	=======================				*/
	r = i = 0;

	while	( ++i < argc-1 ) {
		r |= copy((src_entry = makentry(pname(argv[i]), 
			dname(argv[i]))), dst_entry);
		freetree(src_entry);
	}
	return(r);

usage:
	fprintf(stderr, "usage: %s f1 f2; || cp f1 ... fn d1\n", PRMPT);
	return(1);
}

copy(sourceptr, targetptr)	/*		copy file		*/
struct	entry	*sourceptr;	/*	Source file entry pointer	*/
struct	entry	*targetptr;	/*	Destination file entry pointer	*/
{
	struct	entry	*tempptr;/*	Temporary file entry pointer	*/
	register	int	srclun, trglun, n;
	register	unsigned short mode;

	/*	Test if source-file can be opened			*/
	if	((srclun = open(sourceptr->e_fname, 0)) < 0) {

		/*	cannot open file				*/
		fprintf(stderr, NOOPEN, sourceptr->e_fname);
		return(1);
	}

	/*	Determine file-status					*/
	if (getstat(sourceptr) == NULL) {
		fprintf(stderr, NOACC, sourceptr->e_fname);
		return(1);
	}
	mode	=	sourceptr->e_stat->st_mode;

	getstat(targetptr);

	/*	Is target-file a directory then add source dname	*/
	if	(targetptr->e_stat && isdir(*(targetptr->e_stat))) {
		tempptr = targetptr;
		targetptr = makentry(tempptr->e_fname, 
			dname(sourceptr->e_fname));
		getstat(targetptr);
	}

	/*	Test if the target-file already exists and has the	*/
	/*	same physical space as the source-file			*/
	if	(targetptr->e_stat != NULL &&
		sourceptr->e_stat->st_dev == targetptr->e_stat->st_dev &&
		sourceptr->e_stat->st_ino == targetptr->e_stat->st_ino) {
		fprintf(stderr, NOCPSF, targetptr->e_fname);
			close(srclun);
			return(1);
	}

	/*	Test if target-file can be created			*/
	if	((trglun = creat(targetptr->e_fname, mode)) < 0) {
		/*	cannot create file				*/
		fprintf(stderr, NOCREA, targetptr->e_fname);
		close(srclun);
		return(1);
	}

	/*		COPY FILES				*/

	while	(n = read(srclun, cpbuf, BUFSIZ)) {

		/*	Test if read error			*/
		if	(n < 0) {
			fprintf(stderr, RDERR, sourceptr->e_fname);
			close(srclun);
			close(trglun);
			return(1);
		}
		else if	(write(trglun, cpbuf, n) != n) {
			fprintf(stderr, WRERR, targetptr->e_fname);
			close(srclun);
			close(trglun);
			return(1);
		}
	}
	/*	OK all complete, no errors			*/
	close(srclun);
	close(trglun);
	return(0);
}
