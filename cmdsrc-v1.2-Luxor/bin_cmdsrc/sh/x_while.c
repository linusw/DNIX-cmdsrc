/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	==================================
*
*	== x_while.c ==	Execute while node
*
*	==================================
*
*	Edit 001	1983-07-12
*/

#include	"defs.h"
extern	int	dead_end;
extern	int	contlevel;
extern	int	breaklevel;

int	x_while(node, input, output)
register WHILPTR node; register int input, output;
BEGIN
    register int a, d = dead_end;
    
PRINT("X_WHILE\n");
    dead_end = FALSE;
    WHILE x_cmdlist(node->whilcond, input, output) == 0 DO
        a = x_cmdlist(node->whildo, input, output);
	IF breaklevel THEN
		--breaklevel;
		break;
	ELIF contlevel THEN
		IF --contlevel THEN
			break;
		FI
	FI
    OD
    dead_end = d;
    return(a);
END
