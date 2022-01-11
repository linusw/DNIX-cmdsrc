/* chkstr.c */

/*
 * Check that a string just contains alpha-numeric characters or underscores.
 * If so, return zero, else non-zero.
 */

/* 1984-02-16, Peter Andersson, Luxor Datorer AB */

int
chkstr(s)
register	char	*s;	/* string to check */
{
	while (*s != '\0' && ((*s >= 'a' && *s <= 'z') || (*s >= '0' && *s <= '9') || *s == '_')) {
		s++;
	}
	return(*s);
}
