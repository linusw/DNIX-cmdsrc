/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	===================================
*
*	== x_case.c ==	Execute 'case' node
*
*	===================================
*
*	Edit 001	1983-07-18
*/

#include	"defs.h"

extern	char	*cmdsub();
extern	char	*varsub();
extern	char	*unquote();
extern	int	wmatch();
extern	int	x_cmdlist();

int x_case(node, input, output)
register CASEPTR node; register int input, output;
BEGIN
    register BRCHPTR bp;
    register WORDPTR wp;
    register BRCHPTR  match = (BRCHPTR)NIL;
    register char    *var;
    
PRINT("X_CASE\n");
    var = unquote(cmdsub(varsub(node->casevar)));

    FOR bp = node->caselist; bp ANDF !match; bp = bp->brchnxt DO
        FOR wp = bp->brchkeys; wp ANDF !match; wp = wp->wordnxt DO
            IF wmatch(var, cmdsub(varsub(wp->wordtxt)), TRUE) THEN
                match = bp;
            FI
        OD
    OD
    
    IF match THEN
        return(x_cmdlist(match->brchdo, input, output) );
    ELSE
        return(0);
    FI
END
