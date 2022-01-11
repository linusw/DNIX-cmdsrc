/* getgroup.c */

/*
 * This routine gets a group name and id from the standard input.
 * The name and id are returned as strings and the function returns
 * the group id.
 */

/* 1984-02-23, Peter Andersson, Luxor Datorer AB */

#include	<stdio.h>
#include	<grp.h>
#include	"msg.h"

int
getgroup(bufn, bufid, cmd, flg)
register	char	*bufn;		/* returned name                  */
char			*bufid;		/* returned id                    */
register	char	*cmd;		/* command name                   */
register	int	*flg;		/* add line in /etc/group if != 0 */
{
	register	int	flgleg;		/* legal name flag       */
	register	int	flgacc;		/* accepted flag         */
	int			gid;		/* group id              */
	struct	group	*getgrent();		/* next in /etc/group    */
	struct	group	*getgrnam();		/* search for match name */
	struct	group	*grp;			/* pntr to grp structure */
	struct	group	*getgrgid();		/* search for match gid  */
	static	char	grpnam[] = GROUPNAME;
	static	char	grpid[] = GROUPID;

	do {				/* until legal name */
		accline(bufn, cmd, grpnam);
		if (*bufn) {		/* if not empty */
			clowstr(bufn);		/* convert to lower case */
		}
		else {
			strcpy(bufn, "other");		/* default */
			printf("%s%s\n", grpnam, bufn);
		}
		if (flgleg = !strcmp(bufn, "?")) {
			while ((grp = getgrent()) != NULL) {
				printf("%s\n", grp->gr_name);
			}
			endgrent();		/* close /etc/group */
		}
		else if (flgleg = chkstr(bufn)) {
			cmderr(1, cmd, ILLGRPNAME);
		}
	} while (flgleg);		/* until legal name */

	if ((grp = getgrnam(bufn)) != NULL) {
		*flg = 0;		/* don't add line in /etc/group */
		sprintf(bufid, "%d", gid = grp->gr_gid);
		printf("%s%s\n", grpid, bufid);
	}
	else {
		*flg = 1;		/* add line in /etc/group */
		do {			/* until accepted */
			do {		/* until legal */
				accline(bufid, cmd, grpid);	/* get gid */
				if (flgleg = (*bufid)) {
					if (flgleg = (chkint(&bufid, &gid) || gid <= 0 || gid >= 256)) {
						cmderr(1, cmd, ILLGROUPID);
					}
				}
				else {
					gid = freegid(bufid);
					printf("%s%s\n", grpid, bufid);
				}
			} while (flgleg);		/* until legal */
			if (flgacc = (getgrgid(gid) != NULL)) {
				cmderr(2, cmd, bufid, ALRDYUSED);
			}
		} while (flgacc);			/* until accepted */
	}
	return(gid);
}
