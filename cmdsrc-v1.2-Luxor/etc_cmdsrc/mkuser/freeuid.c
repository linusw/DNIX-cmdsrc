/* freeuid.c */

/*
 * Find the smallest free (not used) user id. The user id is returned
 * together with a string containing it too.
 */

/* 1984-02-23, Peter Andersson, Luxor Datorer AB */

#include	<stdio.h>
#include	<pwd.h>

int
freeuid(buf)
register	char	*buf;	/* receives the uid as a string */
{
	struct	passwd	*getpwuid();	/* find matching uid */
	register	int	uid;	/* user id           */

	for (uid = 1 ; getpwuid(uid) != NULL ; uid++)
		;
	sprintf(buf, "%d", uid);
	return(uid);
}
