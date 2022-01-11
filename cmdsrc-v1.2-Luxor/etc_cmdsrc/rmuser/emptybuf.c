/* emptybuf.c */

/*
 * Empty the standard input buffer until '\n' or EOF.
 */

/* 1984-02-20, Peter Andersson, Luxor Datorer AB */

#include	<stdio.h>

emptybuf(c)
register	int	c;		/* last char read */
{
	while (c != '\n' && c != EOF) {
		c = getchar();
	}
	return;
}
