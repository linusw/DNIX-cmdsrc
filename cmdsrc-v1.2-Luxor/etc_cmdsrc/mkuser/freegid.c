/* freegid.c */

/*
 * Find the smallest free (not used) group id in the /etc/group file.
 * The group id is returned together with a string containing it too.
 */

/* 1984-02-23, Peter Andersson, Luxor Datorer AB */

#include	<stdio.h>
#include	<grp.h>

int
freegid(buf)
register	char	*buf;	/* receives the gid string */
{
	struct	group	*getgrgid();	/* find matching gid */
	register	int	gid;	/* group id          */

	for (gid = 1 ; getgrgid(gid) != NULL ; gid++)
		;
	sprintf(buf, "%d", gid);
	return(gid);
}
