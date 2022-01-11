/* getinid.c */

/*
 * Get initial working directory. A pointer to a legal pathname is returned.
 */

/* 1984-02-20, Peter Andersson, Luxor Datorer AB */

#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	"msg.h"

char *
getinid(buf, cmd, creatflg)
register	char	*buf;		/* input buffer                */
register	char	*cmd;		/* command name                */
register	int	*creatflg;	/* != 0 if dir must be created */
{
	register	int	flgleg;		/* legal dir name flag */
	register	int	flgacc;		/* accepted dir flag   */
	register	int	a;		/* various uses        */
	register	char	*pnt;		/* temporary pointer   */
	struct		stat	stbuf;		/* to hold inode info  */

	do {				/* loop until accepted */
		do {			/* loop until legal */
			accline(buf, cmd, INITDIR);
			emptychk(buf, cmd);	/* exit if empty line */
			clowstr(buf);		/* cnvrt to low case */
			if (flgleg = (*buf != '/')) {
				cmderr(1, cmd, USEFULLPATH);
			}
			else if ((*creatflg = flgleg = strcmp(buf, "/")) && (flgleg = chkpath(buf))) {
				cmderr(1, cmd, ILLPATHNAME);
			}
		} while (flgleg);	/* until legal */
		for (pnt = buf ; *pnt != '\0' ; pnt++)
			;
		for ( ; *pnt != '/' ; pnt--)	/* remove last */
			;
		*pnt = '\0';
		if (pnt != buf && (flgacc = (stat(buf, &stbuf) || (stbuf.st_mode & S_IFMT) != S_IFDIR))) {
			cmderr(2, cmd, buf, DONTEXIST);
			*pnt = '/';
		}
		else {
			*pnt = '/';
			if (flgacc = (((*creatflg = stat(buf, &stbuf)) == 0)
			    && (stbuf.st_mode & S_IFMT) != S_IFDIR)) {
				cmderr(2, cmd, buf, NOTDIR);
			}
			else if (!(*creatflg)) {
				do {	/* until YES or NO */
					printf(buf);
					printf(ALRDYDIR);
					a = getchar();
					emptybuf(a);
					flgacc = (a != UYES && a != LYES);
				} while (flgacc && a != UNO && a != LNO);
			}
		}
	} while (flgacc);		/* until accepted */
	return(buf);
}
