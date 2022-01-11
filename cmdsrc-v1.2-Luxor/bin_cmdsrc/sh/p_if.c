/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	====================================
*
*	== p_if ==	Parse if-statement
*
*	====================================
*
*	Edit 002	1983-07-15
*/
#include	"defs.h"
#include	"cmdbuf.h"

extern	int	p_rsrv();
extern	NODPTR	p_cmdlist();
extern	char	*getstack();
extern		unexp();


NODPTR p_if()
BEGIN
    register IFPTR	p;
    register NODPTR	c1,c2,c3 = (NODPTR)NIL;
    register int	rcode, pos;
    
PRINT("P_IF\n");
    IF  (c1 = p_cmdlist(SUBCMD) )
        ANDF
        (pos = cmdpos(), p_rsrv(TRUE) == R_THEN ? TRUE : (unexp(pos), FALSE)  )
        ANDF
        (c2 = p_cmdlist(SUBCMD) )
        ANDF
        (   (pos = cmdpos(), (rcode = p_rsrv(TRUE) ) == R_FI )
            ORF
            (   (rcode == R_ELSE ? TRUE : (unexp(pos), FALSE) )
                ANDF
                (c3 = p_cmdlist(SUBCMD) )
                ANDF
                (pos = cmdpos(), p_rsrv(TRUE) == R_FI ? TRUE : (unexp(pos),FALSE) )
            )
        )
    THEN
        p = (IFPTR)getstack(IFSIZ);
        p->iftype = IFTYPE;
        p->ifnxt  = (NODPTR)NIL;
        p->ifback = FALSE;
        p->ifcond = c1;
        p->ifthen = c2;
        p->ifelse = c3;
        return( (NODPTR)p );
    ELSE
        return( (NODPTR)NIL );
    FI
END
