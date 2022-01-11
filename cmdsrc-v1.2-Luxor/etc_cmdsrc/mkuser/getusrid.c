/* getusrid.c */

/*
 * This function gets a user id from the keyboard (standard input), checks
 * that it is legal, and not already used.
 * A pointer to the user id (as a string) is returned.
 */

/* 1984-02-20, Peter Andersson, Luxor Datorer AB */

#include	<stdio.h>
#include	<pwd.h>
#include	"msg.h"

char *
getusrid(buf, cmd, uidp)
char			*buf;		/* pointer to digit string */
register	char	*cmd;		/* command name            */
register	int	*uidp;		/* pointer to user id      */
{
	struct	passwd	*getpwuid();	/* find user id function */
	register	flgleg;		/* various uses          */
	register	flgacc;		/* accepted flag         */

	do {				/* until user id accepted */
		do {			/* until user id ok */
			accline(buf, cmd, USERID);
			if (flgleg = (*buf)) {		/* if not empty */
				flgleg = chkint(&buf, uidp);
				if (flgleg = (flgleg || *uidp <= 0 || *uidp >= 256)) {
					cmderr(1, cmd, ILLUSERID);
				}
			}
			else {
				*uidp = freeuid(buf);	/* find free uid */
				printf(USERID);
				printf(buf);
				putchar('\n');
			}
		} while (flgleg);	/* until user id ok */
		if (flgacc = (getpwuid(*uidp) != NULL)) {
			cmderr(2, cmd, buf, ALRDYUSED);	/* already used */
		}
	} while (flgacc);		/* until accepted id */
	return(buf);
}
