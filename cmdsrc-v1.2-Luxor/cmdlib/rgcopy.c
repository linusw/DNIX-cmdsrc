/*	Function rgcopy - regular copy or link/unlink of files		*/
/*	======================================================		*/
/*									*/
/*	Copyright (C) DIAB T{by 1983					*/
/*	Written by: Per-Arne Svensson 					*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include "../cmd.h"
#include "../cmd_err.h"

/*		External functions					*/
/*		==================					*/

struct	entry *makentry();	/*	Make one tree entry		*/
struct	entry *freetree();	/*	Free memory occupied by tree	*/
struct	stat  *getstat();	/*	Make one status block entry	*/

rgcopy(src_ptr, dst_ptr, newflg, linkflg)
struct	entry	*src_ptr;	/*	Source file entry pointer	*/
struct	entry	*dst_ptr;	/*	Destination file entry pointer	*/
int	newflg, linkflg;
{
int	___res = 0;		/*	Return variable			*/

	if (src_ptr == NULL || dst_ptr == NULL) {
		___res++;
		goto exit;
	}

	/*	Test if source file is accessible			*/
	if (getstat(src_ptr) == NULL) {
		/*	Cannot access source file			*/
		fprintf(stderr, NOACC, src_ptr->e_fname);
		___res++;
		goto exit;
	}
	/*	Test if file already exists				*/
	if (getstat(dst_ptr) != NULL) {
		if (newflg) {	/*	Newflag				*/
			fprintf(stderr, NOOVWR, dst_ptr->e_fname);
			___res++;
			goto exit;
		}

		/*	Test if the names are linked to the same file	*/

		if ((src_ptr->e_stat)->st_dev == (dst_ptr->e_stat)->st_dev) {
			if ((src_ptr->e_stat)->st_ino == 
				(dst_ptr->e_stat)->st_ino)
				goto exit;

			if (linkflg && unlink(dst_ptr->e_fname) == -1) {
				fprintf(stderr, NOUNLN, dst_ptr->e_fname);
				___res++;
				goto exit;
			}
		}
	}

	/*	Test linkflag					*/
	if (linkflg) {
		if (link(src_ptr->e_fname, dst_ptr->e_fname) != -1)
			goto exit;
	}
	___res = fcopy(src_ptr, dst_ptr, (src_ptr->e_stat)->st_mode);
exit:
	/*	Remove destination status block			*/
	if (dst_ptr->e_stat != NULL) {
		free(dst_ptr->e_stat);
		dst_ptr->e_stat = NULL;
	}
	return(___res);
error:
	return(1);
}

static fcopy(src_ptr, dst_ptr, mode)		/*	copy file	*/
struct	entry *src_ptr, *dst_ptr;
unsigned short mode;
{
	char	cpbuf[BUFSIZ];
	register	int	srclun, trglun, n;

	/*	Test if source-file can be opened			*/
	if	((srclun = open(src_ptr->e_fname, 0)) < 0) {

		/*	cannot open file				*/
		fprintf(stderr, NOOPEN, src_ptr->e_fname);
		return(1);
	}

	/*	Test if target-file can be created			*/
	if	((trglun = creat(dst_ptr->e_fname, mode)) < 0) {
		/*	cannot create file				*/
		fprintf(stderr, NOCREA, dst_ptr->e_fname);
		close(srclun);
		return(1);
	}

	/*		COPY FILES				*/

	while	(n = read(srclun, cpbuf, BUFSIZ)) {

		/*	Test if read error			*/
		if	(n < 0) {
			fprintf(stderr, RDERR, src_ptr->e_fname);
			close(srclun);
			close(trglun);
			return(1);
		}
		else if	(write(trglun, cpbuf, n) != n) {
			fprintf(stderr, WRERR, dst_ptr->e_fname);
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
