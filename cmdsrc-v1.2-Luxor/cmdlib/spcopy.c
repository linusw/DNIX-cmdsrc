/*	Function for copying special files - spcopy			*/
/*	===========================================			*/
/*									*/
/*	Copyright (C) DIAB T{by 1983					*/
/*	Written by: Per-Arne Svensson 					*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dnix/fcodes.h>
#include "../cmd.h"
#include "../cmd_err.h"

spcopy(src_ptr, dst_ptr)
struct	entry	*src_ptr;	/*	Source file entry pointer	*/
struct	entry	*dst_ptr;	/*	Destination file entry pointer	*/
{
struct	stat  *getstat();	/*	Make one status block entry	*/
int	lu_in, lu_out;		/*	Logic unit numbers		*/
int	___res = 0;		/*	Return variable			*/
register char *name = src_ptr->e_fname;
char	buf[512];		/*	i/o buffer			*/

	if (src_ptr == NULL || dst_ptr == NULL ||
		getstat(src_ptr) == NULL)
		return(1);

	if ((lu_in = svc(F_LOCATE, *name == DELIM 
		? RDIRLU : CDIRLU, name, strlen(name), 0, 0, 0, 0)) < 0 ||
		svc(F_IOCR, lu_in, buf, sizeof(buf), 
		SF_DEVP, 0, 0, 0) < 0 ||
		(lu_out = creat(dst_ptr->e_fname, 0777)) < 0 ||
		write(lu_out, buf, sizeof(buf)) != sizeof(buf)) {
		fprintf(stderr, NOCREA, dst_ptr->e_fname);
		___res = 1;
	} else {
		svc(F_CLOSE, lu_in);
		svc(F_CLOSE, lu_out);
		___res = 0;
	}

	if (chmod(dst_ptr->e_fname, src_ptr->e_stat->st_mode) != 0) {
		fprintf(stderr, NOCHMOD, dst_ptr->e_fname);
		unlink(dst_ptr->e_fname);
		___res = 1;
	}

	/*	Remove destination status block			*/
	if (dst_ptr->e_stat != NULL) {
		free(dst_ptr->e_stat);
		dst_ptr->e_stat = NULL;
	}	
	return(___res);
}
