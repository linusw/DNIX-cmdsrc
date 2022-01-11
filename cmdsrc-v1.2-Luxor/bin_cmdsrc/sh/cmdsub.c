/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	======================================
*
*	== cmdsub.c ==	`command` substitution
*
*	======================================
*
*	Edit 001	1983-08-25
*/

#include "defs.h"

extern	int	parsex();
extern	char	*getstack();
extern  char	charin();
extern	int	instr();
extern		strcpy();
extern	char	*leftstr();
extern	int	strlen();
extern	char	*conc3();
extern	int	interactive;
extern	char	*ldpchar;
extern	char	*unquote();

char *cmdsub(s)
register char *s;
BEGIN
    char *listen();
    register int p1, p2;
    
PRINT("CMDSUB\n");
    IF  (p1 = instr(s, C_CMDSUB) ) >= 0
        ANDF
        (p2 = instr(s+p1+1, C_CMDSUB) ) >= 0
    THEN
        p2 += p1+1;
        s = conc3(leftstr(s,p1-1), listen(leftstr(s+p1+1, p2-p1-2)), cmdsub(s+p2+1));
    FI
    return(s);
END

char *listen(s)		/*  Execute command s in a subshell,  */
register char *s;	/*  and return its output.            */
BEGIN
    register int pipefd1[2], pipefd2[2], child_id, buflen;
    register char *p, *buffer, *tmp;
    
PRINT("LISTEN\n");
    IF pipe(pipefd1) == -1			/*  Start two pipes  */
        ORF
        pipe(pipefd2) == -1
        ORF
        (child_id = fork() ) == -1		/*  and fork  */
    THEN
        perror("shell");
        return("");
    ELIF child_id == 0 THEN
        clfile(0);				/*  child  */
        clfile(1);				/*---------*/
        dup2(pipefd1[0], 0);
        dup2(pipefd2[1], 1);
        clfile(pipefd1[1]);
        clfile(pipefd2[0]);
        interactive = FALSE;
        parsex(0, SUBCMD);
        exit(0);
        
    ELSE
        putwait(child_id);
        clfile(pipefd1[0]);			/*----------*/
        clfile(pipefd2[1]);			/*  parent  */
        buflen = 255;				/*----------*/
        p = buffer = getstack(buflen+1);
	s = unquote(s);
        write(pipefd1[1],s, strlen(s) );
        write(pipefd1[1],"\n",1);
        clfile(pipefd1[1]);
	WHILE (*p++ = charin(pipefd2[0])) != C_EOF DO /* Read child's output */
            IF p == buffer + buflen THEN
                *p = C_NUL;
                tmp = getstack((buflen += 256) +1);
                strcpy(tmp, buffer);		/*  Get more buffer */
                p = p-buffer+tmp;		/*  when needed     */
                buffer = tmp;
            FI
        OD
        p = p-2;
        clfile(pipefd2[0]);
        WHILE *p == C_NL DO --p OD		/*  Strip trailing newlines. */
        *++p = C_NUL;				/*  Terminate.               */
        FOR p=buffer; *p; ++p DO		/*  Replace newlines with    */
            IF *p == C_NL THEN *p = C_SP FI	/*  blanks.                  */
        OD
        return(buffer);
    FI
END
