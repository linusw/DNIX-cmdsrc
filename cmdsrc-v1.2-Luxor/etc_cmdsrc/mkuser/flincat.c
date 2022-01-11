/* flincat.c */

/*
 * This function copies a file to another and appends one line to it,
 * while doing extensive error checking.
 * Zero is returned if everything is ok, 1 if the destination file can't
 * be created, or 2 if another error occurs.
 * Note that the line terminator is supposed to be a newline (which is
 * also written.
 */

/* 1984-02-29, Peter Andersson, Luxor Datorer AB */

#include	"defs.h"

int
flincat(toname, fromname, line, buf, maxbuf)
register	char	*toname;	/* destination file name */
register	char	*fromname;	/* source file name      */
register	char	*line;		/* line to be appended   */
register	char	*buf;		/* file copy buffer      */
register	int	maxbuf;		/* size of buf           */
{
	register	int	fdfrom;	/* source file descriptor      */
	register	int	fdto;	/* destination file descriptor */

	if ((fdfrom = open(fromname, 0)) < 0) {
		return(2);		/* can't open source */
	}
	if ((fdto = creat(toname, FIL_PROT)) < 0) {
		close(fdfrom);
		return(1);		/* can't create destination */
	}
	if (fcopy(fdto, fdfrom, buf, maxbuf) || flinwrit(fdto, line)) {
		close(fdfrom);
		close(fdto);
		unlink(toname);		/* remove destination */
		return(2);		/* can't copy or append */
	}
	close(fdfrom);
	if (close(fdto)) {
		unlink(toname);
		return(2);		/* can't close destination */
	}
	return(0);			/* everything ok */
}
