/* getline.c */

/*
 * This function reads a string , terminated by a newline, from the standard
 * input. The length (including the terminating 0 byte) of the read
 * string is returned (0 if the string was too long, EOF if end of file).
 */

/* 1984-02-16, Peter Andersson, Luxor Datorer AB */

#include	<stdio.h>

int
getline(buf, maxlen)
register	char	*buf;		/* storage buffer */
register	int	maxlen;		/* size of buf    */
{
	register	int	c;		/* last read character */
	register	int	len = 1;	/* length of input     */

	while ((c = getchar()) == ' ' || c == '\t')	/* skip blanks */
		;
	while (c != '\n' && c != EOF)	{
		*buf++ = c;
		len++;
		if (len >= maxlen) {
			emptybuf(c);		/* empty input buffer */
			return(0);		/* too long input */
		}
		c = getchar();
	}
	while (len > 1 && (*buf == ' ' || *buf == '\t')) {
		buf--;
		len--;
	}
	*buf = '\0';
	return((c == EOF) ? EOF : len);
}
