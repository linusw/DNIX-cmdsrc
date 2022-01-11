/* clowstr.c */

/*
 * This routine converts all alphabetic upper case letters in a string
 * to lower case.
 */

/* 1984-02-17, Peter Andersson, Luxor Datorer AB */

clowstr(s)
char	s[];		/* string to convert */
{
	register	int	i;		/* loop index */

	for (i = 0 ; s[i] != '\0' ; i++) {
		if (s[i] >= 'A' && s[i] <= 'Z') {
			s[i] = s[i] + 'a' - 'A';
		}
	}
	return;
}
