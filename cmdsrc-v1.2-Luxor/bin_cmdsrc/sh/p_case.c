/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	======================================
*
*	== p_case.c ==	Parse 'case' statement
*
*	======================================
*
*	Edit 002	1983-07-19
*
*	Modified by P-A Svensson
*	Dataindustrier AB T{by
*	Apr 1984
*/

#include	"defs.h"
#include	"cmdbuf.h"


extern		unexp();
extern	int	p_rsrv();
extern	char	*getstack();
extern	char	*secvar;
extern	char	*p_word();
extern	NODPTR	p_cmdlist();


NODPTR p_case()
BEGIN
    int			ok;
    register int	pos;
    register char	*var;
    register CASEPTR	cp = (CASEPTR)NIL;
    register BRCHPTR	list;
    BRCHPTR		p_caselist();
    
PRINT("P_CASE\n");
    IF  (pos = cmdpos(), (int)*(var = p_word(TRUE) ) ? TRUE : (unexp(pos), FALSE) )
        ANDF
        (pos = cmdpos(), p_rsrv(TRUE) == R_IN ? TRUE : (unexp(pos), FALSE) )
        ANDF
        (list = p_caselist(&ok), ok)
    THEN
        cp = (CASEPTR)getstack(CASESIZ);
        cp->casetype = CASETYPE;
        cp->casenxt  = (NODPTR)NIL;
        cp->caseback = FALSE;
        cp->casevar  = var;
        cp->caselist = list;
    FI
    return( (NODPTR)cp );
END


BRCHPTR p_caselist(ok)
register int	*ok;
BEGIN
    register BRCHPTR bp = (BRCHPTR)NIL;
    WORDPTR  p_pattern();
    register WORDPTR keys;
    register NODPTR  np = (NODPTR)NIL;
    register int     pos, rcode;
    
PRINT("P_CASELIST\n");
    skipblanks(secvar);
    pos = cmdpos();
    SWITCH rcode = p_rsrv(TRUE) IN
        CASE R_ESAC:
            *ok = TRUE;
            BREAK
        CASE 0: 
            keys = p_pattern(ok);
            IF  *ok THEN
                pos = cmdpos(); rcode = p_rsrv(TRUE);
                IF rcode != R_ENDCAS ANDF rcode != R_ESAC THEN
                    setcmdpos(pos);
                    np = p_cmdlist(SUBCAS);
                    pos = cmdpos(); rcode = p_rsrv(TRUE);
                FI
                SWITCH rcode IN
                    CASE R_ESAC:
                        setcmdpos(pos);
                    CASE R_ENDCAS:
                        bp = (BRCHPTR)getstack(BRCHSIZ);
                        bp->brchkeys = keys;
                        bp->brchdo   = np;
                        bp->brchnxt  = p_caselist(ok);
                        BREAK
                    DEFAULT:
                        unexp(pos);
                        *ok = FALSE;
                ENDSW
            FI
            BREAK
        DEFAULT:
            unexp(pos);
            *ok = FALSE;
    ENDSW
    return(bp);
END


WORDPTR p_pattern(ok)
register int	*ok;
BEGIN
    register int	pos, rcode;
    register char	*s;
    register WORDPTR	wp;
    
PRINT("P_PATTERN|n");
    *ok = FALSE;
    IF *(s = p_word(TRUE) ) THEN
        wp = (WORDPTR)getstack(WORDSIZ);
        wp->wordtxt = s;
        IF pos = cmdpos(), (rcode = p_rsrv(FALSE) ) == R_RGR THEN
            wp->wordnxt = (WORDPTR)NIL;
            *ok = TRUE;
        ELIF rcode == R_PIPE ? TRUE : (unexp(pos), FALSE) THEN
            wp->wordnxt = p_pattern(ok);
        FI
    FI
    return(wp);
END
