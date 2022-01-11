
/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	=======================================
*
*	== mess.c ==	Message output routines
*
*	=======================================
*
*	Edit 001	1983-05-18
*/

#include "defs.h"
#include "cmdbuf.h"

extern	char	*p_word();
extern	int	p_rsrv();
extern	char	*getrsrv();
extern	char	readchar();
extern 	int	interactive;
extern	jmp_buf	mainenv;

unexp(pos)
register int	pos;
BEGIN
    register char	*text;
    register int	rcode;
    
    setcmdpos(pos);
    SWITCH rcode = p_rsrv(FALSE) IN
        CASE R_NL:
            text = ER_NL;
            BREAK
        DEFAULT:
            text = getrsrv(rcode);
            BREAK
        CASE 0:
            IF *(text = p_word(TRUE) ) == C_NUL THEN
                SWITCH readchar("") IN
                    CASE C_EOF:
                        text = ER_EOF;
                ENDSW
            FI
    ENDSW
    fatalerr(text);
END

fatalerr(s)
char *s;
BEGIN
    fprintf(stderr,"%s '%s' %s\n", ER_SYNT, s, ER_UNXP);
    IF interactive THEN
	longjmp(mainenv,0);
    FI
    exit(1);
END

prline(s)
register char *s;
BEGIN
    fprintf(stderr,"%s\n",s);
END

prbrk(s)
register char *s;
BEGIN
    fprintf(stderr,"%s",s);
END

errline(s)
register char *s;
BEGIN
    fprintf(stderr,"%s\n",s);
END

errbrk(s)
register char *s;
BEGIN
    fprintf(stderr,"%s",s);
END

prtime(t)			/*  Print time. t in 1/60 second  */
register long int t;
BEGIN
    register int h, m, s;

    t = (t + 30)/60;		/*  Round to whole seconds  */
    s = t%60; t /= 60;
    m = t%60;
    IF h = t/60 THEN
        fprintf(stderr,"%dh",h);
    FI
    fprintf(stderr,"%dm%ds ",m,s);
END
