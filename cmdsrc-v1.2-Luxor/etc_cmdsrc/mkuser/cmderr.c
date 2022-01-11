/* cmderr.c */

/*
 * Display command error message.
 */

/* 1984-02-17, Peter Andersson, Luxor Datorer AB */

#include	<stdio.h>

cmderr(n, cmd, s1, s2)
register	int	n;		/* no. of strings after ':' */
register	char	*cmd;		/* command name             */
register	char	*s1;		/* error message            */
register	char	*s2;		/* error message            */
{
	fprintf(stderr, "%s: %s ", cmd, s1);
	if (n > 1) {
		fprintf(stderr, "%s", s2);
	}
	putc('\n', stderr);
	return;
}
