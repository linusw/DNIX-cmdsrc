/*	Function fetching USER- or GROUP-number				*/
/*	=======================================				*/
/*									*/
/*	Copyright (C) DIAB T{by 1984					*/
/*	Written by: Per-Arne Svensson 					*/

#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

IDnum(thisID, grpflg)
register char *thisID;		/*	The current  UID/GID-name	*/
register int grpflg;
{
struct passwd	*getpwnam(), *pwdres;	/*	Get password record	*/
struct group	*getgrnam(), *gidres;	/*	Get group record	*/
static	int	lastnum = -1;		/*	Last user-/group-number	*/
static	int	lastflg;		/*	Last grpflg-value	*/
static	char	lastID[10]="";		/*	Last fetched UID/GID-name*/

	if (strncmp(thisID, lastID, sizeof(lastID)) || grpflg != lastflg) {
		strncpy(lastID, thisID, sizeof(lastID));
		lastflg = grpflg;
		grpflg ? setgrent() : setpwent();
		if (grpflg) {
			gidres = getgrnam(thisID);
			lastnum = gidres ? gidres->gr_gid : -1;
		} else {
			pwdres = getpwnam(thisID);
			lastnum = pwdres ? pwdres->pw_uid : -1;
		}
	}
	return(lastnum);
}
