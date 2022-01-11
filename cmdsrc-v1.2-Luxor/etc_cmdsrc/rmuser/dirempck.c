/* dirempck.c */

/*
 * This function checks if a directory is empty (excluding '.', '..', and
 * '.profile') or not.
 * If it is empty, zero is returned, else non-zero.
 */

/* 1984-03-13, Peter Andersson, Luxor Datorer AB */

#include	<sys/types.h>
#include	<sys/dir.h>
#include	"defs.h"

int
dirempck(name)
register	char	*name;		/* pointer to dir name to check */
{
	register	int	fd;		/* file descriptor     */
	register	int	empflg = 0;	/* empty flag          */
	struct		direct	dirbuf;		/* dir entry structure */

	if ((fd = open(name, 0)) == -1) {	/* open directory */
		return(0);			/* no dir present */
	}
	while (read(fd, (char *)&dirbuf, sizeof(dirbuf)) > 0) {
		if (dirbuf.d_ino && strcmp(dirbuf.d_name, ".") && strcmp(dirbuf.d_name, "..")
		    && strcmp(dirbuf.d_name, PROFILE)) {
			empflg = 1;		/* it wasn't empty */
			break;
		}
	}
	close(fd);
	return(empflg);
}
