/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	================================================
*
*	== p_io.c ==		parse input-output
*
*	================================================
*
*	Edit 013	1983-07-17
*
*	Modified by P-A Svensson
*	Dataindustrier AB T{by
*	Apr 1984
*/


#include	"defs.h"
#include	"cmdbuf.h"


extern	char	*cmdcop();
extern		skipblen();
extern	char	readchar();
extern	char	*p_word();
extern	int	p_rsrv();
extern	int	strequ();
extern	char	*getstack();
extern	char	*secvar;
extern	char	*cmdsub();
extern	char	*varsub();
extern	char	*unquote();
extern	int	interactive;
extern		unexp();

IOPTR p_io(fd1)
register int	fd1;
BEGIN
    register int code,start;
    char *p_hdoc();
    register IOPTR p;
    
PRINT("P_IO\n");
    skipblen("");
    start=cmdpos();
    p=(IOPTR)getstack(IOSIZ);
    p->ionxt=(IOPTR)NIL;
    p->iotxt=(char *)0;

    code = p_rsrv(FALSE);
    SWITCH code IN
        CASE R_INP:
        CASE R_OUT:
            IF code == R_INP THEN
                p->iotype=INPTYPE;
                p->iofd1 = (fd1 >=0 ? fd1 : 0);
            ELSE
                p->iotype = OUTTYPE;
                p->iofd1 = (fd1 >= 0 ? fd1 : 1);
            FI
            IF *(p->iotxt = p_word(TRUE)) == C_NUL THEN 
                setcmdpos(start);
                p = (IOPTR)NIL;
            FI
            BREAK
            
        CASE R_INDUP:
        CASE R_OUTDUP:
            IF code == R_INDUP THEN
                p->iotype = INDUPTYPE;
                p->iofd1 = (fd1 >= 0 ? fd1 : 0 );
            ELSE
                p->iotype = OUTDUPTYPE;
                p->iofd1 = (fd1 >= 0 ? fd1 : 1);
            FI
            IF (p->iofd2 = p_int(FDMIN,FDMAX) ) < 0 THEN
                p = (IOPTR)NIL;
                setcmdpos(start);
            FI
            BREAK
        CASE R_CLINP:
        CASE R_CLOUT:
            IF code == R_CLINP THEN
                p->iotype = CLINPTYPE;
                p->iofd1 = (fd1 >= 0 ? fd1 : 0 );
            ELSE
                p->iotype = CLOUTTYPE;
                p->iofd1 = (fd1 >= 0 ? fd1 : 1);
            FI
            BREAK
        CASE R_APP:
            IF *(p->iotxt = p_word(TRUE) ) == C_NUL THEN
                p = (IOPTR)NIL;
                setcmdpos(start);
		BREAK
            FI
            p->iotype=APPTYPE;
            p->iofd1 = (fd1>= 0 ? fd1 : 1);
            BREAK
            
        CASE R_HDOC:
            IF  p->iohdoc = p_hdoc() THEN
                p->iotype = HDOCTYPE;
                p->iofd1  = (fd1 >= 0 ? fd1 : 0);
            ELSE
                p = (IOPTR)NIL;
                setcmdpos(start);
            FI
            BREAK
        DEFAULT:
            p = (IOPTR)NIL;
            setcmdpos(start);
    ENDSW
    return (p);
    
END

char *p_hdoc()
BEGIN
    register    char c = ' ', *s ,*delim, *prompt = secvar;
    register    int start, stop;
    
PRINT("P_HDOC\n");
    IF *(delim = p_word(TRUE) ) == C_NUL THEN
        unexp(cmdpos() );
        return(FALSE);
    ELSE
        start = cmdpos() +1;
        REPEAT
            WHILE (c = readchar(prompt) ) != C_NL ANDF c != C_EOF DO OD
            stop = cmdpos();
            s = p_word(TRUE);
        UNTIL strequ(unquote(s),unquote(delim)) ORF c == C_EOF DONE
	IF *delim & QUOTE THEN
		return(unquote(cmdcop(start, stop, TRUE)));
	FI
        return(unquote(cmdsub(varsub(cmdcop(start, stop, TRUE)))));
    FI
END
