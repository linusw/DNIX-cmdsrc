/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	=====================================
*
*	== p_for.c ==	Parse 'for' statement
*
*	=====================================
*
*	Edit 001	1983-07-16
*/

#include	"defs.h"
#include	"cmdbuf.h"


extern	char	*p_word();
extern		unexp();
extern	int	p_rsrv();
extern	NODPTR	p_cmdlist();
extern	char	*getstack();


NODPTR p_for()
BEGIN
    register int	pos, rcode;
    register char	*var;
    WORDPTR  p_wlst();
    register WORDPTR wlst = (WORDPTR)NIL;
    register NODPTR	cp;
    register FORPTR	p;

PRINT("P_FOR\n");
    IF  (pos = cmdpos(), (int)(var = p_word(TRUE) ) ? TRUE : (unexp(pos), FALSE) )
        ANDF
        (   (pos = cmdpos(), (rcode = p_rsrv(TRUE) ) == R_DO )
            ORF
            (   (rcode == R_IN ? (int)(wlst = p_wlst() ) : (unexp(pos), FALSE) )
                ANDF
                (pos = cmdpos(), p_rsrv(TRUE) == R_DO ? TRUE : (unexp(pos), FALSE) )
            )
        )
        ANDF
        (cp = p_cmdlist(SUBCMD) )
        ANDF
        (pos = cmdpos(), p_rsrv(TRUE) == R_DONE ? TRUE : (unexp(pos), FALSE) )
    THEN
        p = (FORPTR)getstack(FORSIZ);
        p->fortype = FORTYPE;
        p->fornxt  = (NODPTR)NIL;
        p->forback = FALSE;
        p->forvar  = var;
        p->forwlst = wlst;
        p->fordo   = cp;
        return( (NODPTR)p );
    ELSE
        return( (NODPTR)NIL );
    FI
END

WORDPTR p_wlst()
BEGIN
    register char	*w;
    register int	pos, rcode;
    register WORDPTR	firstptr = (WORDPTR)NIL, lastptr;
    
PRINT("P_WLST\n");
    IF pos = cmdpos(), *(w = p_word(TRUE) ) THEN
        firstptr = lastptr = (WORDPTR)getstack(WORDSIZ);
        lastptr->wordtxt = w;
        lastptr->wordnxt = (WORDPTR)NIL;
        WHILE *(w = p_word(TRUE) ) DO
            lastptr = lastptr->wordnxt = (WORDPTR)getstack(WORDSIZ);
            lastptr->wordtxt = w;
            lastptr->wordnxt = (WORDPTR)NIL;
        OD
    FI
    IF  firstptr
        ANDF
        (pos = cmdpos(), (rcode = p_rsrv(FALSE) ) == R_SEP ORF rcode == R_NL )
    THEN
        return(firstptr);
    ELSE
        unexp(pos);
        return( (WORDPTR)NIL );
    FI
END
