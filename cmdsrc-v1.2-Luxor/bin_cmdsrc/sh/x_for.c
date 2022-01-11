/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	==================================
*
*	== x_for.c ==	Execute 'for' node
*
*	==================================
*
*	Edit 001	1983-07-16
*/

#include	"defs.h"

extern		setvar();
extern	int	x_cmdlist();
extern	WORDPTR	posparlst();
extern	int	contlevel;
extern	int	breaklevel;
extern	int	dead_end;
extern	WORDPTR	argcrunch();

int	x_for(node, input, output)
register FORPTR node;
register int	input, output;
BEGIN
    register WORDPTR varnode; int d = dead_end;

PRINT("X_FOR\n");
    IF node->forwlst THEN
        varnode = argcrunch(node->forwlst);
    ELSE
        varnode = posparlst();
    FI
    
    FOR ; varnode ; varnode = varnode->wordnxt DO
        setvar(node->forvar, varnode->wordtxt);
        dead_end = FALSE;    
        x_cmdlist(node->fordo, input, output);
        dead_end = d;
        IF breaklevel THEN
            --breaklevel;
            return(0);
        ELIF contlevel THEN
            IF --contlevel THEN
                return(0);
            FI
        FI
    OD
    END
