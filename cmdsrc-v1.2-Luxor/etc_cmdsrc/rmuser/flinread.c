/* flinread.c */

/*
 * This function opens a file for reading. The file descriptor is
 * returned, or a negative number if the open is unsuccessful.
 */

/* 1984-03-09, Peter Andersson, Luxor Datorer AB */

#include	"defs.h"

/*
 * Static areas used as buffers and pointers.
 */

static	char	fbuf[BUFSIZE];
static	char	*bufp = fbuf;		/* pointer into file buffer       */
static	int	n = 0;			/* remaining characters in buffer */

int
ropen(filnam)
register	char	*filnam;	/* name of file to open */
{
	n = 0;				/* no chars in buffer */
	return(open(filnam, 0));
}

/*
 * This function reads a line from a file while doing extensive error
 * checking.
 * The number of characters read is returned (including the newline).
 * If end of file is reached, 0 is returned. If an error occurs, a
 * negative value is returned.
 * Note that the line will not be null-terminated.
 */

int
flinread(fd, buf, maxbuf)
register	int	fd;		/* file descriptor     */
register	char	*buf;		/* buffer to read into */
register	int	maxbuf;		/* size of buf         */
{
	static		int	err;		/* error flag     */
	register	int	len = 0;	/* loop index     */
	register	char	c;		/* read character */

	do {
		if (n == 0) {
			if ((err = n = read(fd, fbuf, BUFSIZE)) <= 0) {
				break;
			}
			else {
				bufp = fbuf;
			}
		}
		c = *bufp++;
		*(buf + (len++)) = c;
		n--;
	} while (c != '\n' && len < maxbuf);
	return((err <= 0) ? err : ((c == '\n') ? len : (-1)));
}
