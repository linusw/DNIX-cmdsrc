/*
 *	Routines for administrating the group file.
 *
 *	Copyright (C) DIAB T{by 1984
 *	Written by: Per-Arne Svensson 
 */

#include <stdio.h>
#include <signal.h>
#include <grp.h>
#include <sys/types.h>
#include "../cmd.h"
#include "../cmd_err.h"


/*	Function for appending an entry to the group file appgrent()	*/
/*	============================================================	*/
appgrent(grp)
register struct	group *grp;
{
int	putgrent(), res;
static	char	grpfile[] = GROUP_FILE;
register FILE	*grp_str;
	/*	If the group file doesn't exist, create it		*/
	if (access(grpfile, 0) == -1 && close(creat(grpfile, 644)) == -1) {
		fprintf(stderr, NOCREA, grpfile);
		return(-1);
	}

	/*	Test if the file can be opened for writing		*/
	/*	Protect the group file from being written by someone	*/
	if (mk_lock(GROUP_LOCK, 240) == -1)
		return(-1);

	if ((grp_str = fopen(grpfile, "a")) == NULL) {
		fprintf(stderr, NOOPEN, grpfile);
		rmv_lock(GROUP_LOCK);
		return(-1);
	}

	res = putgrent(grp, grp_str);

	/*	Close group file					*/
	fclose(grp_str);

	/*	Reset the group file status				*/
	rmv_lock(GROUP_LOCK);

	return(res);
}

