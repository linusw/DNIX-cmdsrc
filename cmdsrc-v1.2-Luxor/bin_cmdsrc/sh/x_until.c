/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	==================================
*
*	== x_until.c ==	Execute until node
*
*	==================================
*
*	Edit 001	1983-07-16
*/

#include	"defs.h"
extern	int	dead_end;
extern	int	contlevel;
extern	int	breaklevel;

int	x_until(node, input, output)
register UNTILPTR node; register int input, output;
BEGIN
    register int a, d = dead_end;

PRINT("X_UNTIL\n");
    dead_end = FALSE;    
    WHILE x_cmdlist(node->untilcond, input, output) != 0 DO
        a = x_cmdlist(node->untildo, input, output);
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
