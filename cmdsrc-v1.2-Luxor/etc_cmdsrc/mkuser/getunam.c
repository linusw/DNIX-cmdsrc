/* getunam.c */

/*
 * This routine gets and returns a pointer to a legal user name.
 */

/* 1984-02-17, Peter Andersson, Luxor Datorer AB */

#include	<stdio.h>
#include	<pwd.h>
#include	"defs.h"
#include	"msg.h"

char *
getunam(buf, dname, argc, cmd)
register	char	*buf;		/* input buffer           */
register	char	*dname;		/* default user name      */
register	int	argc;		/* command argument count */
register	char	*cmd;		/* command name           */
{
	register	char	*rname;		/* returned name       */
	register	int	len;		/* length of read name */
	register	int	flgleg;		/* legal name flag     */
	register	int	flgacc;		/* accepted name flag  */
	struct	passwd	*getpwnam();		/* find user name      */

	do {			/* loop until accepted name */
		do {		/* loop until legal name */
			if (argc == 2) {
				rname = dname;	/* use command arg */
			}
			else {
				accline(buf, cmd, USERNAME);
				emptychk(buf, cmd);
				rname = buf;
			}
			clowstr(rname);		/* convert to lower case */
			flgleg = chkstr(rname);
			if (flgleg) {
				cmderr(1, cmd, ILLUSERNAME);
				if (argc == 2) {
					exit(1);	/* ill cmd arg */
				}
			}
		} while (flgleg);	/* until legal name */

		if (flgacc = (getpwnam(rname) != NULL)) {
			cmderr(2, cmd, rname, ALRDYUSED);
			if (argc == 2) {
				exit(1);	/* already used, exit */
			}
		}
	} while (flgacc);		/* until accepted name */
	return(rname);
}
