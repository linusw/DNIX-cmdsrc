/*
 *	Routines for administrating the group file.
 *
 *	Copyright (C) DIAB T{by 1984
 *	Written by: Per-Arne Svensson 
 */

#include <stdio.h>
#include <sys/types.h>
#include <grp.h>

/*	Function for writing an entry to the group file putgrent()	*/
/*	==========================================================	*/
putgrent(grp, str)
register struct group *grp;
register FILE	*str;
{
	fprintf(str, "%s:%s:%d:",
	grp->gr_name,				/*	Group name	*/
	grp->gr_passwd,				/*	Group password	*/
	grp->gr_gid);				/*	Group ID	*/

	{register char **mbptr;
		mbptr = grp->gr_mem;
		while (*mbptr) {
			fprintf(str,"%s", *mbptr);/*	List of members	*/
			fprintf(str,(*++mbptr) ? "," : "\n");
		}
	}
}
