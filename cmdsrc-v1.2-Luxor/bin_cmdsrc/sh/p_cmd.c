/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	===============================
*
*	== p_cmd.c ==	parse a command
*
*	===============================
*
*	Edit 010	1983-07-12
*/

#include	"defs.h"
#include	"cmdbuf.h"

extern	int	p_rsrv();
extern	char	*getrsrv();
extern	NODPTR	p_cmdlist();
extern	NODPTR	p_for();
extern	NODPTR	p_while();
extern	NODPTR	p_until();
extern	NODPTR	p_case();
extern	NODPTR	p_if();
extern	NODPTR	p_simple();

NODPTR p_cmd(must)			/* If must is TRUE then abscence of */
register int must;
BEGIN					/* a command is an error.           */
    register int		start, code, pos;
    register NODPTR	cp = (NODPTR)NIL;
    register SUBPTR	sp;
    
PRINT("P_CMD\n");
    start=cmdpos();
    SWITCH code = p_rsrv(FALSE) IN
        CASE R_LGR:						/*  '('  */
        CASE R_BGN:						/*  'begin'  */
            IF cp = p_cmdlist(SUBCMD) THEN
                pos = cmdpos();
                IF p_rsrv(TRUE) == ((code == R_BGN) ? R_END : R_RGR) THEN
                    sp = (SUBPTR)getstack(SUBSIZ);
                    sp->subtype = SUBTYPE;
                    sp->subnxt = (NODPTR)NIL;
                    sp->subback = FALSE;
                    sp->subsub = (code == R_LGR);
                    sp->subdo   = cp;
                    return((NODPTR)sp);
                ELSE
                    unexp(pos);
                    return((NODPTR)NIL);
                FI
            FI
            return((NODPTR)NIL);
        CASE R_FOR:							/*  'for'  */
            return(p_for() );
        CASE R_WHI:							/*  'while'  */
            return(p_while() );
        CASE R_UNT:							/*  'until'  */
            return(p_until() );
        CASE R_CAS:							/*  'case'  */
            return(p_case() );
        CASE R_IF:							/*  'if'  */
            return(p_if() );
        CASE R_INP:
        CASE R_OUT:
        CASE R_HDOC:
        CASE R_APP:
        CASE R_INDUP:
        CASE R_OUTDUP:
        CASE R_CLINP:
        CASE R_CLOUT:							/*  These are handled by p_simple  */
            setcmdpos(start);
        CASE 0:								/*  No reserved word --> simple cmd  */
            return(p_simple() );
        DEFAULT:							/*  Other reserved words  */
            IF must THEN
                unexp(start);
            FI
            setcmdpos(start);
            return((NODPTR)NIL);
    ENDSW
END
