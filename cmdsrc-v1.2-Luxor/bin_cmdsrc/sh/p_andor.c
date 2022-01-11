/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	=====================================================
*
*	== p_andor.c ==		Parse an andor
*
*	=====================================================
*
*	Edit 005	1983-07-14
*/

#include	"defs.h"
#include	"cmdbuf.h"

extern	int	p_rsrv();
extern NODPTR	p_pipe();


NODPTR p_andor(must)
register int	must;
BEGIN
    register ANDORPTR p = (ANDORPTR)NIL;
    register NODPTR lptr,rptr;
    register int pos, type, rcode;
    
PRINT("P_ANDOR\n");
    IF lptr = p_pipe(must) THEN
        
        pos = cmdpos();
        rcode = p_rsrv(FALSE);
        SWITCH rcode IN
            CASE R_ANDF:
                type=ANDTYPE;
                BREAK
            CASE R_ORF:
                type=ORTYPE;
                BREAK
            DEFAULT:
                setcmdpos(pos);
                return(lptr);
                BREAK
        ENDSW
        
        IF rptr = p_andor(TRUE) THEN
            
            p = (ANDORPTR)getstack(ANDORSIZ);
            p->andortype  = type;
            p->andorback  = FALSE;
            p->andorleft  = lptr;
            p->andorright = rptr;
        FI
        return( (NODPTR)p );
    FI
    return( (NODPTR)NIL);
END
