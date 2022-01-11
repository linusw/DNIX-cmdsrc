/* chkint.c */

/*
 * Check that a string is an unsigned integer.
 * If it isn't an integer, return a non-zero value.
 */

/* 1984-02-20, Peter Andersson,Luxor Datorer AB */

int
chkint(bufp, valp)
register	char	**bufp;		/* pointer to string    */
register	int	*valp;		/* pointer to int value */
{
	register	int	i;		/* loop index             */
	register	int	temp;		/* used to check overflow */
	register	char	*tmppnt;	/* temp string pointer    */

	*valp = 0;			/* init accumulator */
	tmppnt = *bufp;
	if (*tmppnt < '0' || *tmppnt > '9') {
		return(1);		/* no digits */
	}
	while (*tmppnt >= '0' && *tmppnt <= '9') {
		temp = *valp;
		for (i = 1 ; i < 10 ; i++) {
			*valp += temp;
			if (*valp < 0) {
				return(1);	/* overflow */
			}
		}
		*valp += (*tmppnt++) - '0';
		if (*valp < 0) {
			return(1);		/* overflow */
		}
	}
	if (*tmppnt != '\0') {
		return(1);			/* trailing chars */
	}
	while (**bufp == '0') {			/* skip leading zeros */
		(*bufp)++;
	}
	if (**bufp == '\0') {
		(*bufp)--;			/* just zeroes */
	}
	return(0);
}
