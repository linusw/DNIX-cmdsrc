/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	=====================================
*
*	== p_while.c ==	parse while statement
*
*	=====================================
*
*	Edit 002	1983-07-15
*/

#include	"defs.h"
#include	"cmdbuf.h"

extern	int	p_rsrv();
extern	NODPTR	p_cmdlist();
extern		unexp();



NODPTR	p_while()
BEGIN
    register WHILPTR	p;
    register NODPTR	c1, c2;
    register int	pos;
    
PRINT("P_WHILE\n");
    IF (c1 = p_cmdlist(SUBCMD) )
        ANDF 
        (pos = cmdpos(), p_rsrv(TRUE) == R_DO ? TRUE : (unexp(pos),FALSE) )
        ANDF 
        (c2 = p_cmdlist(SUBCMD) )
        ANDF 
        (pos = cmdpos(), p_rsrv(TRUE) == R_DONE ? TRUE : (unexp(pos), FALSE) )
    THEN
        p           = (WHILPTR)getstack(WHILSIZ);
        p->whiltype = WHILTYPE;
        p->whilnxt  = (NODPTR)NIL;
        p->whilback = FALSE;
        p->whilcond = c1;
        p->whildo   = c2;
        return( (NODPTR)p );
    ELSE
        return( (NODPTR)NIL );
    FI
END
