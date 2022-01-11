/*	Function fetching USER- or GROUP-name				*/
/*	=====================================				*/
/*									*/
/*	Copyright (C) DIAB T{by 1983					*/
/*	Written by: Per-Arne Svensson 					*/

#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

char *
IDname(thisID, grpflg)
register int thisID;		/*	The current  UID/GID	*/
register int grpflg;
{
struct passwd	*getpwuid(), *pwdres;	/*	Get password record	*/
struct group	*getgrgid(), *gidres;	/*	Get group record	*/
static	char	lastname[10];		/*	Last user-/group-name	*/
static	int	 lastID = -1;		/*	Last fetched UID/GID	*/
static	int	lastflg;		/*	Last grpflg-value	*/

	if (thisID != lastID || grpflg != lastflg) {
		lastID = thisID;
		lastflg = grpflg;
		grpflg ? setgrent() : setpwent();
		if (grpflg) {
			gidres = getgrgid(thisID);
			gidres ? sprintf(lastname, "%s", gidres->gr_name) : 
				sprintf(lastname, "%d", thisID);
		} else {
			pwdres = getpwuid(thisID);
			pwdres ? sprintf(lastname, "%s", pwdres->pw_name) : 
				sprintf(lastname, "%d", thisID);
		}
	}
	return(lastname);
}
