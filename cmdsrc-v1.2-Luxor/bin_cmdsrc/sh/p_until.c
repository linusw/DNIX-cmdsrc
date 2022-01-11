/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	=====================================
*
*	== p_until.c ==	parse until statement
*
*	=====================================
*
*	Edit 001	1983-07-16
*/

#include	"defs.h"
#include	"cmdbuf.h"

extern	int	p_rsrv();
extern	NODPTR	p_cmdlist();
extern		unexp();



NODPTR	p_until()
BEGIN
    register UNTILPTR	p;
    register NODPTR	c1, c2;
    register int	pos;
    
PRINT("P_UNTIL\n");
    IF (c1 = p_cmdlist(SUBCMD) )
        ANDF 
        (pos = cmdpos(), p_rsrv(TRUE) == R_DO ? TRUE : (unexp(pos),FALSE) )
        ANDF 
        (c2 = p_cmdlist(SUBCMD) )
        ANDF 
        (pos = cmdpos(), p_rsrv(TRUE) == R_DONE ? TRUE : (unexp(pos), FALSE) )
    THEN
        p           = (UNTILPTR)getstack(UNTILSIZ);
        p->untiltype = UNTILTYPE;
        p->untilnxt  = (NODPTR)NIL;
        p->untilback = FALSE;
        p->untilcond = c1;
        p->untildo   = c2;
        return( (NODPTR)p );
    ELSE
        return( (NODPTR)NIL );
    FI
END
