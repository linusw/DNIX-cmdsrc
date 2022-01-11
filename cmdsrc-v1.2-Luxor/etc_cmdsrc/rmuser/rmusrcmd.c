/* rmusrcmd.c */

/*
 * This file contains the source code for the main function of the rmuser
 * command.
 */

/* 1984-03-05, Peter Andersson, Luxor Datorer AB */

#include	<stdio.h>
#include	<signal.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	"defs.h"
#include	"msg.h"

main(argc, argv)
int	argc;			/* argument count        */
char	*argv[];		/* pointers to arguments */
{
	char		buf[BSIZE];		/* input buffer            */
	char		*dirnam;		/* pointer to dir name     */
	char		filbuf[FBSIZE];		/* file buffer             */
	register char	*strp;			/* string pointer          */
	register char	*tmpp;			/* pointer to temp name    */
	register	i, ii, j;		/* indicies                */
	register	fdfrom;			/* file descr, source      */
	register	fdto;			/* file descr, destination */
	register	len;			/* string length           */
	int		csave;			/* character save          */
	int		isave;			/* index save              */
	int		save;			/* various uses            */
	int		ropt;			/* r option flag           */
	int		nopt;			/* n option flag           */
	int		uid;			/* user id to remove       */
	struct	stat	stbuf;			/* file status structure   */
	char		*getusr();		/* get user name           */
	static	char	pwdfil[] = PWDFIL;
	static	char	cantup[] = CANTUPDATE;
	static	char	cantcr[] = CANTCREATE;
	static	char	grpfil[] = GRPFIL;
	static	char	lost[] = LOST;
	static	char	illopt[] = ILLOPTION;
	static	char	control[] = "%s: %s %s %s\n";

	/*
	 * Find the options specified.
	 */
	ropt = nopt = 0;
	for (i = 1 ; i < argc ; i++) {
		if (*argv[i] != '-') {
			break;
		}
		else {
			for (j = 1 ; *(argv[i] + j) != '\0' ; j++) {
				if (*(argv[i] + j) == 'r') {
					ropt = 1;
				}
				else if (*(argv[i] + j) == 'n') {
					nopt = 1;
				}
				else {
					cmderr(1, argv[0], illopt);
					exit(1);
				}
			}
		}
	}
	if (ropt && nopt) {
		cmderr(1, argv[0], illopt);
		exit(1);
	}
	if ((argc += 1 - i) > 2) {
		cmderr(1, argv[0], TOOMANYARGS);
		exit(1);
	}
	if (getuid()) {				/* if not super user */
		cmderr(1, argv[0], NOTSUPER);
		exit(1);
	}

	/*
	 * Get user name to remove
	 */
	strp = getusr(buf, argv[i], argc, argv[0], &dirnam, &uid);

	/*
	 * Ignore some signals.
	 */
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGALRM, SIG_IGN);

	/*
	 * Remove user from /etc/passwd file.
	 */
	tmpp = TMPPWDFIL;
	if ((fdfrom = ropen(pwdfil)) < 0) {
		cmderr(2, argv[0], cantup, pwdfil);
		exit(1);			/* can't open /etc/passwd */
	}
	if ((fdto = creat(tmpp, FIL_PROT)) < 0) {
		cmderr(2, argv[0], cantcr, tmpp);
		exit(1);			/* can't create temp file */
	}
	while ((len = flinread(fdfrom, filbuf, FBSIZE)) > 0) {
		for (i = 0 ; filbuf[i] != ':' && filbuf[i] != '\n' ; i++)
			;
		csave = filbuf[i];
		filbuf[i] = '\0';		/* extract user name */
		j = strcmp(filbuf, strp);	/* name to remove? */
		filbuf[i] = csave;		/* restore */
		if (!j) {
			continue;		/* it was user to remove */
		}
		if ((len = flinwrit(fdto, filbuf)) < 0) {
			break;			/* write error */
		}
	}	/* while (len = */
	close(fdfrom);				/* close /etc/passwd */
	if (close(fdto) || len < 0) {		/* if any error */
		unlink(tmpp);			/* remove temp file */
		cmderr(2, argv[0], cantup, pwdfil);
		exit(1);			/* can't update /etc/passwd */
	}

	/*
	 * Replace old /etc/passwd file by the updated one.
	 */
	if (i = chname(pwdfil, tmpp)) {		/* replace old passwd */
		if (i == 1) {
			cmderr(2, argv[0], cantup, pwdfil);
		}
		else {
			fprintf(stderr, control, argv[0], pwdfil, lost, tmpp);
		}
		exit(1);
	}

	/*
	 * Remove user from /etc/group file.
	 */
	tmpp = TMPGRPFIL;
	if ((fdfrom = ropen(grpfil)) < 0) {
		cmderr(2, argv[0], cantup, grpfil);
		exit(1);			/* can't open /etc/group */
	}
	if ((fdto = creat(tmpp, FIL_PROT)) < 0) {
		cmderr(2, argv[0], cantcr, tmpp);
		exit(1);			/* can't create temp file */
	}
	while ((len = flinread(fdfrom, filbuf, FBSIZE)) > 0) {
		for (i = 0 ; filbuf[i] != ':' && filbuf[i] != '\n' ; i++)
			;
		isave = i;			/* save index */
		csave = filbuf[i];		/* save character */
		filbuf[i] = '\0';		/* extract group name */
		if (csave == ':') {
			for (i++ ; filbuf[i] != ':' && filbuf[i] != '\n' ; i++)
				;
		}
		if (filbuf[i] == ':') {
			for (i++ ; filbuf[i] != ':' && filbuf[i] != '\n' ; i++)
				;
		}
		if (filbuf[i] == ':') {
			i++;
		}
		for (ii = i ; filbuf[i] != '\n' ; ) {
			save = i;
			for (j = 0 ; filbuf[i] == (*(strp + j)) ; i++, j++)
				;
			if (filbuf[i] != ',' && filbuf[i] != '\n') {
				for (i = save ; (filbuf[ii] = filbuf[i]) != ',' && filbuf[i] != '\n' ;
				     i++, ii++)
					;
				ii++;
			}
			if (filbuf[i] == ',') {
				i++;
			}
		}	/* for (ii = */
		if (filbuf[ii-1] == ',') {
			ii--;
		}
		filbuf[ii] = '\n';
		if (save = (filbuf[--ii] == ':')) {
			do {			/* until YES or NO */
				printf(GRPEMPTY, filbuf);
				i = getchar();
				emptybuf(i);
				save = (i == UYES || i == LYES);
			} while (!save && i != UNO && i != LNO);
		}
		filbuf[isave] = csave;		/* restore */
		if (!save && (len = flinwrit(fdto, filbuf)) < 0) {
			break;			/* write error */
		}
	}	/* while (len = */
	close(fdfrom);				/* close /etc/group */
	if (close(fdto) || len < 0) {		/* if any error */
		unlink(tmpp);			/* remove temp file */
		cmderr(2, argv[0], cantup, pwdfil);
		exit(1);			/* can't update /etc/group */
	}

	/*
	 * Replace old /etc/group by the new one.
	 */
	if (i = chname(grpfil, tmpp)) {		/* change to new group */
		if (i == 1) {
			cmderr(2, argv[0], cantup, grpfil);
		}
		else {
			fprintf(stderr, control, argv[0], grpfil, lost, tmpp);
		}
		exit(1);
	}

	/*
	 * Delete the users directory.
	 */
	strcpy(filbuf, dirnam);
	if (stat(filbuf, &stbuf) == -1) {
		exit(0);			/* no directory present */
	}
	if ((stbuf.st_mode & S_IFMT) != S_IFDIR) {
		exit(0);			/* not a directory */
	}
	if (stbuf.st_uid != uid) {
		exit(0);			/* not owned by user */
	}
	if (!nopt) {
		if (!ropt && dirempck(filbuf)) {
			cmderr(2, argv[0], filbuf, NOTEMPTY);
			exit(1);
		}
		else if (deldir(filbuf, FBSIZE)) {
			cmderr(2, argv[0], FAILED, filbuf);
			exit(1);
		}
		else {
			printf(REMOVED, filbuf);
		}
	}

	/*
	 * Ready (and everything is ok).
	 */
	exit(0);
}
