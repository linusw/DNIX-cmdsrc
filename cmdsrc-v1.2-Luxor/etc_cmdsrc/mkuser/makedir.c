/* makedir.c */

/*
 * This function creates a directory. If an error occurs, a non-zero value
 * is returned and no directory has been created.
 * Note that the directory name must be a full and legal name, otherwise
 * strange things may happen.
 */

/* 1984-02-28, Peter Andersson, Luxor Datorer AB */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	"defs.h"

int
makedir(name, tmp)
register	char	*name;		/* directory to create           */
register	char	*tmp;		/* temp area, 3 longer than name */
{
	register	int	i, j;		/* string indicies */
	register	int	err;		/* error flag      */
	register	int	csave;		/* character save  */

	if ((err = mknod(name, S_IFDIR | DIR_PROT, 0)) == 0) {
		for (i = 0 ; *(name + i) != '\0' ; i++)	  /* end of name */
			;
		j = i + 2;
		for ( ; *(name + i) != '/' ; i--)	/* find last '/' */
			;
		if (i == 0) {
			i++;		/* just /name */
		}
		csave = *(name + i);
		strcpy(tmp, name);
		strcat(tmp, "/.");	/* construct name of . */
		if (err = link(name, tmp)) {	/* create . */
			unlink(name);		/* remove directory */
		}
		else {
			strcat(tmp, ".");	/* construct name of .. */
			*(name + i) = '\0';	/* real name of .. */
			err = link(name, tmp);	/* create .. */
			*(name + i) = csave;	/* restore name of dir */
			if (err) {
				*(tmp + j) = '\0';	/* name of . */
				unlink(tmp);		/* remove . */
				unlink(name);		/* remove dir */
			}
		}
	}
	return(err);
}
