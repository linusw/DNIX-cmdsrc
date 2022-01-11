/* fcopy.c */

/*
 * This function copies a file to another while doing extensive error
 * checking.
 * A non-zero value is returned if an error occurs.
 */

/* 1984-02-29, Peter Andersson, Luxor Datorer AB */

int
fcopy(fdto, fdfrom, buf, maxbuf)
register	int	fdto;		/* destination file descriptor */
register	int	fdfrom;		/* source file descriptor      */
register	char	*buf;		/* file buffer                 */
register	int	maxbuf;		/* size of buf                 */
{
	register	int	n = 0;		/* # of chars read */
	register	int	err = 0;	/* error flag      */

	while (err == n && (n = read(fdfrom, buf, maxbuf)) > 0) {
		err = write(fdto, buf, n);
	}
	return(n != 0);
}
