/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	===============================
*
*	== x_if.c ==	Execute if-node
*
*	===============================
*
*	Edit 001	1983-07-11
*/

#include	"defs.h"

extern	int	x_cmdlist();
extern	int	dead_end;
x_if(node, input, output)
register IFPTR node; register int input, output;
BEGIN
    register int d = dead_end, value;    

PRINT("X_IF\n");
    dead_end = FALSE;
    IF          x_cmdlist(node->ifcond, input, output) == 0 THEN
        value = ( x_cmdlist(node->ifthen, input, output) );
    ELIF node->ifelse != (NODPTR)NIL THEN
        value = ( x_cmdlist(node->ifelse, input, output) );
    ELSE
        value = 0;
    FI
    dead_end = d;
    return(value);
END
