/* getshell.c */

/*
 * Get a program to be used as shell. A pointer to a legal executable
 * file is returned.
 */

/* 1984-02-22, Peter Andersson, Luxor Datorer AB */

#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	"msg.h"

char *
getshell(buf, cmd)
register	char	*buf;		/* input buffer */
register	char	*cmd;		/* command name */
{
	struct		stat	stbuf;		/* to hold inode info */
	register	int	flgleg;		/* legal flag         */
	register	int	flgacc;		/* accepeted flag     */
	register	int	a;		/* various uses       */

	do {				/* loop until accepted */
		do {			/* loop until legal */
			accline(buf, cmd, SHELL);
			if (flgleg = (*buf)) {		/* if not empty */
				clowstr(buf);	/* cnvrt to lower */
				if (flgleg = (*buf != '/')) {
					cmderr(1, cmd, USEFULLPATH);
				}
				else if (flgleg = chkpath(buf)) {
					cmderr(1, cmd, ILLPATHNAME);
				}
			}
		} while (flgleg);	/* until legal */

		if (flgacc = *buf) {		/* if not empty */
			if (flgacc = stat(buf, &stbuf)) {
				do {		/* until YES or NO */
					printf(buf);
					printf(NOFILE);
					a = getchar();
					emptybuf(a);
					flgacc = (a != UYES && a != LYES);
				} while (flgacc && a != UNO && a != LNO);
			}
			else if (flgacc = ((stbuf.st_mode & S_IFMT) != S_IFREG)) {
				cmderr(2, cmd, buf, NOTPROGRAM);
			}
			else if (flgacc = ((stbuf.st_mode & S_IEXEC) == 0)) {
				cmderr(2, cmd, buf, NOTEXECUTABLE);
			}
		}
	} while (flgacc);	/* until accepeted */
	return(buf);
}
