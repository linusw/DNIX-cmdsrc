/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	============================================
*
*	== p_cmdlist.c ==	parse a	command-list
*
*	============================================
*
*	Edit 007	1983-07-19
*
*	Modified by P-A Svensson
*	Dataindustrier AB T{by
*	Apr 1984
*/

#include	"defs.h"
#include	"cmdbuf.h"


extern	char	*getrsrv();
extern	char	nextchar();
extern	char	*primvar, *secvar;
extern	int	p_rsrv();
extern NODPTR	p_andor();
extern		unexp();
extern		skipblanks();



NODPTR p_cmdlist(level)		/*  level = SUBCAS when parsing a cmdlist  */
register int level;		/*  that is part of a case, = SUBCMD when  */
BEGIN				/*  part of other command, and = TOP when  */
    register char c, *w, *prompt; /*  parsing on top level                 */
    register int pos, rcode;
    register NODPTR firstptr = (NODPTR)NIL, lastptr;

PRINT("P_CMDLIST\n");
    prompt = level == TOP ? primvar : secvar; 
    skipblanks(prompt); 
    IF nextchar(prompt) == C_EOF THEN return((NODPTR)NIL) FI
    IF firstptr = lastptr = p_andor(TRUE) THEN
        REPEAT
            pos = cmdpos();
            SWITCH rcode = p_rsrv(FALSE) IN
                CASE R_BACK :
                CASE R_SEP:
                CASE R_NL:
                    lastptr->nodeback = (rcode == R_BACK);
                    IF level != TOP THEN skipblanks(secvar) FI
                    BREAK
                CASE R_ENDCAS:
                CASE R_ESAC:
		CASE R_RGR:
		CASE R_END:
                    IF level == SUBCAS ORF level == SUBCMD
                    THEN
                        setcmdpos(pos);
                        return(firstptr);
                    FI
		CASE R_EOF:
		    IF level == TOP THEN BREAK FI
                DEFAULT:
                    unexp(pos);
                    return( (NODPTR)NIL );
            ENDSW
        UNTIL ( ( level == TOP ANDF rcode == R_NL)
            ORF !(lastptr = lastptr->nodenxt = p_andor(FALSE) ) )
        DONE
    FI
    return(firstptr);
END
