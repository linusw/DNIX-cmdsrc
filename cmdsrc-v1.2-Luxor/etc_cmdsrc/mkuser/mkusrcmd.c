/* mkusrcmd.c */

/*
 * This file contains the main function for the mkusr command.
 */

/* 1984-02-21, Peter Andersson, Luxor Datorer AB   */
/* 1984-10-02, Peter Andersson added .profile file */

#include	<stdio.h>
#include	<grp.h>
#include	<signal.h>
#include	"defs.h"
#include	"msg.h"

main(argc, argv)
int	argc;		/* argument count        */
char	*argv[];	/* pointers to arguments */
{
	char	pwdstr[CSIZE];		/* /etc/passwd line               */
	char	grpstr[CSIZE];		/* /etc/group line                */
	char	buf[BSIZE + 3];		/* input buffer                   */
	char	buf1[BSIZE];		/* another input buffer           */
	char	dir[BSIZE];		/* input for init dir             */
	char	usr[BSIZE];		/* user name save                 */
	register	i;		/* loop index                     */
	register	psize = 0;	/* length of /etc/passwd line     */
	register	gsize = 0;	/* length of /etc/group line      */
	register char	*strp;		/* next input pointer             */
	register char	*strp1;		/* another next input pointer     */
	register char	*dstrp;		/* pointer to init dir            */
	int		dflg;		/* != 0 if create new dir         */
	int		gflg;		/* != 0 if add line in /etc/group */
	register	gid;		/* group id                       */
	int		uid;		/* user id                        */
	int		cgid;		/* current group id checked       */
	register	fdfrom;		/* source file descriptor         */
	register	fdto;		/* destination file descriptor    */
	char	*getunam();		/* get user name                  */
	char	*getpw();		/* get password                   */
	char	*getusrid();		/* get user id                    */
	char	*getdescr();		/* get optional description       */
	char	*getinid();		/* get initial directory          */
	char	*getshell();		/* get shell program              */
	static	char	pwdfil[] = PWDFIL;
	static	char	grpfil[] = GRPFIL;
	static	char	cantup[] = CANTUPDATE;
	static	char	cantcr[] = CANTCREATE;
	static	char	lost[] = LOST;
	static	char	control[] = "%s: %s %s %s\n";

	if (argc > 2) {
		cmderr(1, argv[0], TOOMANYARGS);   /* too many arguments */
		exit(1);
	}
	if (getuid() != 0) {			   /* if not super user */
		cmderr(1, argv[0], NOTSUPER);
		exit(1);
	}

	/*
	 * Get user name.
	 */
	strp = getunam(buf, argv[1], argc, argv[0]);
	if ((psize = strlen(strp) + 2) >= CSIZE) {
		sizerr(argv[0]);		/* buffer too small */
	}
	strcpy(pwdstr, strp);
	strcat(pwdstr, ":");
	strcpy(usr, strp);			/* save user name */

	/*
	 * Get user id.
	 */
	strp = getusrid(buf, argv[0], &uid);
	if ((psize += strlen(strp) + 1) >= CSIZE) {
		sizerr(argv[0]);		/* buffer too small */
	}

	/*
	 * Get login password.
	 */
	if ((strp1 = getpw(argv[0])) != NULL) {
		if ((psize += strlen(strp1)) >= CSIZE) {
			sizerr(argv[0]);	/* buffer too small */
		}
		strcat(pwdstr, strp1);
	}
	strcat(pwdstr, ":");

	/*
	 * Add user id to string.
	 */
	strcat(pwdstr, strp);
	strcat(pwdstr, ":");

	/*
	 * Get group name and id.
	 */
	gid = getgroup(buf, buf1, argv[0], &gflg);
	if ((psize += strlen(buf1) + 1) >= CSIZE) {
		sizerr(argv[0]);
	}
	strcat(pwdstr, buf1);
	strcat(pwdstr, ":");

	/*
	 * Construct /etc/group line.
	 */
	if (gflg) {				/* if add line */
		if ((gsize = strlen(buf) + 2) >= CSIZE) {
			sizerr(argv[0]);	/* buffer too small */
		}
		strcpy(grpstr, buf);
		strcat(grpstr, ":");

		/*
		 * Get group password.
		 */
		if ((strp = getpw(argv[0])) != NULL) {
			if ((gsize += strlen(strp)) >= CSIZE) {
				sizerr(argv[0]);	/* buffer too small */
			}
			strcat(grpstr,strp);
		}
		strcat(grpstr, ":");

		/*
		 * Add group id to /etc/group line.
		 */
		if (gsize + strlen(buf1) + strlen(usr) >= CSIZE - 2) {
			sizerr(argv[0]);		/* buffer too small */
		}
		strcat(grpstr, buf1);
		strcat(grpstr, ":");
		strcat(grpstr, usr);			/* add user name */
		strcat(grpstr, "\n");
	}

	/*
	 * Get optional description.
	 */
	strp = getdescr(buf, argv[0]);
	if ((psize += strlen(strp) + 1) >= CSIZE) {
		sizerr(argv[0]);			/* buffer too small */
	}
	strcat(pwdstr, strp);
	strcat(pwdstr, ":");

	/*
	 * Get initial working directory.
	 */
	dstrp = getinid(dir, argv[0] , &dflg);
	if ((psize += strlen(dstrp) + 1) >= CSIZE) {
		sizerr(argv[0]);
	}
	strcat(pwdstr, dstrp);
	strcat(pwdstr, ":");

	/*
	 * Get program to use as shell.
	 */
	strp = getshell(buf, argv[0]);
	if (psize + strlen(strp) >= CSIZE) {
		sizerr(argv[0]);
	}
	strcat(pwdstr, strp);
	strcat(pwdstr, "\n");

	/*
	 * Get terminal type.
	 */
	if (dflg) {
		accline(&buf1[0], argv[0], TERMINAL);
		if (buf1[0] == '\0') {
			strp = DEFAULTTERM;	/* use default terminal type */
		}
		else {
			strp = &buf1[0];
		}
	}

	/*
	 * Ignore some signals.
	 */
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGALRM, SIG_IGN);

	/*
	 * Create a new directory (if necessary).
	 */
	if (dflg) {			/* if create */
		if (makedir(dstrp, buf)) {
			cmderr(2, argv[0], cantcr, dstrp);
			exit(1);
		}
		else if (chown(dstrp, uid, gid)) {
			cmderr(2, argv[0], CANTCHANGE, dstrp);
			exit(1);
		}
		printf(CREATMSG, dstrp);

		/*
		 * Create a .profile file.
		 */
		chdir(dstrp);
		i = creat(PROFILE, XFIL_PROT);
		dstrp = PROFTXT1;
		write(i, dstrp, strlen(dstrp));
		write(i, strp, strlen(strp));
		dstrp = PROFTXT2;
		write(i, dstrp, strlen(dstrp));
		close(i);
		chown(PROFILE, uid, gid);
	}

	/*
	 * Append line to /etc/passwd.
	 */
	strp = TMPPWDFIL;
	if (i = flincat(strp, pwdfil, pwdstr, buf, BSIZE)) {
		if (i == 1) {
			cmderr(2, argv[0], cantcr, strp);
		}
		else {
			cmderr(2, argv[0], cantup, pwdfil);
		}
		exit(1);
	}
	if (i = chname(pwdfil, strp)) {		/* replace old passwd */
		if (i == 1) {
			cmderr(2, argv[0], cantup, pwdfil);
		}
		else {
			fprintf(stderr, control, argv[0], pwdfil, lost, strp);
		}
		exit(1);
	}

	/*
	 * Update /etc/group file.
	 */
	strp = TMPGRPFIL;
	if (gflg) {			/* if add line in /etc/group */
		if (i = flincat(strp, grpfil, grpstr, buf, BSIZE)) {
			if (i == 1) {
				cmderr(2, argv[0], cantcr, strp);
			}
			else {
				cmderr(2, argv[0], cantup, grpfil);
			}
			exit(1);
		}
	}
	else {
		if ((fdfrom = ropen(grpfil)) < 0) {
			cmderr(2, argv[0], cantup, grpfil);
			exit(1);	/* can't open /etc/group */
		}
		if ((fdto = creat(strp, FIL_PROT)) < 0) {
			cmderr(2, argv[0], cantcr, strp);
			exit(1);	/* can't create temp file */
		}
		while ((psize = flinread(fdfrom, pwdstr, CSIZE)) > 0) {
			for (i = 0 ; pwdstr[i] != ':' && pwdstr[i] != '\n' ; i++)
				;
			if (pwdstr[i] == ':') {
				for (i++ ; pwdstr[i] != ':' && pwdstr[i] != '\n' ; i++)
					;
			}
			if (pwdstr[i++] == ':') {
				sscanf(pwdstr + i, "%d", &cgid);
				if (cgid == gid) {
					if (psize + strlen(usr) > CSIZE - 1) {
						close(fdto);
						unlink(strp);
						cmderr(2, argv[0], cantup, grpfil);
						exit(1);
					}
					if (pwdstr[--psize - 1] != ':') {
						pwdstr[psize++] = ',';
					}
					for (i = 0 ; usr[i] != '\0' ; i++) {
						pwdstr[psize++] = usr[i];
					}
					pwdstr[psize] = '\n';
				}
			}
			if ((psize = flinwrit(fdto, pwdstr)) < 0) {
				break;		/* write error */
			}
		}
		close(fdfrom);				/* close /etc/group */
		if (close(fdto) || psize < 0) {		/* if any error */
			unlink(strp);			/* remove temp file */
			cmderr(2, argv[0], cantup, grpfil);
			exit(1);			/* can't update /etc/group */
		}
	}
	if (i = chname(grpfil, strp)) {			/* change to new group */
		if (i == 1) {
			cmderr(2, argv[0], cantup, grpfil);
		}
		else {
			fprintf(stderr, control, argv[0], grpfil, lost, strp);
		}
		exit(1);		/* can't rename temp file */
	}

	/*
	 * Ready (and everything is ok).
	 */
	exit(0);
}	/* end of main() */

/*
 * Display a message indicating that enough memory doesn't exist to
 * construct the complete line in the /etc/xxx file. Then exit the
 * command.
 */

sizerr(cmd)
register	char	*cmd;		/* command name */
{
	cmderr(1, cmd, CANTMAKELINE);
	exit(1);
}	/* end of sizerr() */
