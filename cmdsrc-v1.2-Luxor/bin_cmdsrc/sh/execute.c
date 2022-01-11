/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	==============================================
*
*	== execute.c ==	Execute simple command
*
*	==============================================
*
*	Edit 008	1983-07-29
*/

#include "defs.h"

extern	char	**wltostar();
extern	char	*itoa();
extern		setvar();
extern	int	parsex();
extern	char	*getstack();
extern	char	*conc3();
extern	int	interactive;
extern	int	errno;
extern	int	flags;
extern	int	skipex;
extern	char	**environ;
extern  char	*pathvar;
extern	int	dead_end;


/*  If this is not a subshell then assume argvec[0] is the  */
/*      name of a shell procedure, and try to execute it,   */
/*  else assume argvec[0] is the name of an executable      */
/*      a.out type file. Try execv(), and if it fails,      */
/*      take appropriate action.                            */

execute(arglist,subshell)
register WORDPTR arglist; register int subshell;
BEGIN
    register char **argvec, *cmd;

PRINT("EXECUTE\n");
    IF (!arglist) ORF (!(cmd = arglist->wordtxt)) THEN return(0); FI
    argvec = wltostar(arglist);
    IF !subshell THEN return(x_shproc(cmd, argvec, FALSE)); ELSE
	/* try to execute prog, using directory found in PATH */
	register char *p,*dir;
	char *prog,*path,*pathend;

	p = pathvar; path = getstack(strlen(p) + 1);
	strcpy(path,p);
	p=path; pathend = path + strlen(path);
	WHILE *p DO
		IF *p == C_PATHLIM THEN *p = C_NUL; FI
		++p;
	OD
	IF *cmd == '/' THEN dir = pathend; ELSE dir = path; FI
nxdir:
	IF *dir == '\000' THEN
		prog = cmd;
	ELSE
		prog = conc3(dir,"/",cmd);
	FI
	WHILE *dir++ DO OD
	execve(prog, argvec, environ);
        SWITCH errno IN
            CASE ENOENT:					/*  File not found  */
		IF dir < pathend THEN goto nxdir; FI	/* try next */
		errbrk(argvec[0]); errline(ER_NFND);
                return(ENOENT);
            CASE EACCES:					/*  No execute permission  */
		IF skipex THEN
			/* If invoked as 'sh prog...', ignore x-priv */
                	return(x_shproc(prog, argvec, TRUE));
		FI
		IF dir < pathend THEN goto nxdir; FI	/* try next */
		errbrk(argvec[0]);errline(ER_CNEX);
                return(EACCES);
            CASE ENOEXEC:					/*  Bad format - assume shell procedure  */
                return(x_shproc(prog, argvec, TRUE));
		CASE ENOMEM:
			errbrk(argvec[0]); errline(ER_2BIG);
			return(ENOMEM);
		CASE E2BIG:
			errbrk(argvec[0]); errline(ER_ARGL2LNG);
			return(E2BIG);
		CASE ETXTBSY:
			errbrk(argvec[0]); errline(ER_TXTBSY);
			return(ETXTBSY);
            DEFAULT:						/*  Other error  */
		perror(argvec[0]);
                return(-1);
        ENDSW
    FI
END

x_shproc(command, argvec, subshell)
char *argvec[]; register int subshell;
char *command;
BEGIN
    register char **ap;
    register int savint = interactive, value, i;
    int d = dead_end;
    register int file;

PRINT("X_SHPROC\n");
    IF (file = open(command, 0) ) == -1 THEN
        errbrk(argvec[0]); errline(ER_NFND);
        return(-1);
    ELSE
	IF subshell THEN
		kleenex();	/*	Clear non-export variables	*/
	FI
        FOR i=0, ap = &argvec[0]; *ap; ++ap DO	/*  Set positional parameters  */
            setvar(itoa(i++), *ap);
        OD
        setvar(PARVAR,itoa(--i) );		/*  No. of d:o  */
        interactive = FALSE;
	dead_end = FALSE;
        value = parsex(file, subshell?TOP:SUBCMD);
        clfile(file);
        interactive = savint;
	dead_end = d;
        return(value);  
    FI
END

show(list)		/*  Print command  */
WORDPTR list;
BEGIN
    WORDPTR s;
    errbrk("+");
    FOR s=list; s; s = s->wordnxt DO
        errbrk(s->wordtxt); errbrk(" ");
    OD
    errline("");
END
