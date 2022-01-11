/* flinwrit.c */

/*
 * This function writes a line to a file while doing extensive error
 * checking. The line to write must be newline terminated. Also the
 * newline is written.
 * Zero is returned if everything is ok, otherwise a negative value.
 */

/* 1984-02-29, Peter Andersson, Luxor Datorer AB */

int
flinwrit(fd, buf)
register	int	fd;		/* file descriptor */
register	char	*buf;		/* line to write   */
{
	register	int	i;	/* character index */

	for (i = 0 ; *(buf + (i++)) != '\n' ; )
		;
	return((write(fd, buf, i) == i) ? 0 : (-1));
}
