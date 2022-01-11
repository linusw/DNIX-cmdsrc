/* getusr.c */

/*
 * This functions gets a legal and existing user name and returns a
 * pointer to it.
 */

/* 1984-03-01, Peter Andersson, Luxor Datorer AB */

#include	<stdio.h>
#include	<pwd.h>
#include	"defs.h"
#include	"msg.h"

char *
getusr(buf, dname, argc, cmd, dirnam, uidp)
register	char	*buf;		/* input buffer            */
register	char	*dname;		/* default user name       */
register	int	argc;		/* command argument count  */
register	char	*cmd;		/* command name            */
register	char	**dirnam;	/* directory name returned */
register	int	*uidp;		/* returned user id        */
{
	register	char	*rname;		/* returned name        */
	register	int	len;		/* length of read name  */
	struct		passwd	*getpwnam();	/* find user name       */
	struct		passwd	*stpw;		/* pntr to passwd struc */

	if (argc == 2) {
		rname = dname;		/* use command arg */
	}
	else {
		printf(USERNAME);
		len = getline(buf, BSIZE);	/* get user name */
		if (len == EOF) {
			exit(1);		/* exit command */
		}
		else if (!len) {
			cmderr(1, cmd, TOOLONGSTRING);
			exit(1);
		}
		else if (*buf == '\0') {
			cmderr(1, cmd, NOREMOVED);
			exit(0);
		}
		rname = buf;
	}
	clowstr(rname);			/* convert to lower case */
	if (chkstr(rname)) {
		cmderr(1, cmd, ILLUSERNAME);
		exit(1);
	}
	if ((stpw = getpwnam(rname)) == NULL) {
		cmderr(2, cmd, rname, NOTUSER);
		exit(1);
	}
	*dirnam = stpw->pw_dir;
	*uidp = stpw->pw_uid;
	return(rname);
}
