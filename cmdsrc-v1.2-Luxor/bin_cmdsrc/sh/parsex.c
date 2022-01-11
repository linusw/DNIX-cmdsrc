/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	===============================================================
*
*	== parsex.c ==		Parse and execute commands from file stream str
*
*	===============================================================
*
*	Edit 002	1983-07-21
*
*	Modified by P-A Svensson
*	Dataindustrier AB T{by
*	Apr 1984
*/

#include	"defs.h"
#include	"cmdbuf.h"

extern		clearstack();
extern		clearcmdbuf();
extern	NODPTR	p_cmdlist();
extern		x_cmdlist();
extern	char	*primvar;
extern	char	nextchar();
extern	int	flags;
	int	contlevel;
	int	breaklevel;

parsex(file, level)
register int	file;
register int  level;
BEGIN
    register NODPTR  p;
    register char	*prompt;
    register int     done=FALSE;
    register	int	oldin;
    register int      result=0;

PRINT("PARSEX\n");
    oldin = infile;
    infile  = file;
    setvar(PROCVAR, itoa(getpid() ) );
    REPEAT
        IF level == TOP THEN
             clearstack();
             breaklevel = 0;
             contlevel = 0;
        FI
        clearcmdbuf();
        prompt = primvar;
        skipblen(prompt);
        IF  (p = p_cmdlist(TOP) ) ANDF !(flags&N_FLAG) THEN
            result = x_cmdlist(p, 0, 1);
        ELIF nextchar(prompt) == C_EOF THEN
            done = TRUE;
        FI
        IF flags&T_FLAG THEN done = TRUE FI
    UNTIL done DONE
    infile = oldin;
    return(result);
END
