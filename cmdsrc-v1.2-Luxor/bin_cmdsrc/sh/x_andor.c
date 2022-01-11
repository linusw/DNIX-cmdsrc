/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	==========================================
*
*	== x_andor ==		Execute andor node
*
*	==========================================
*
*	Edit 001	1983-07-08
*/

#include	"defs.h"

extern	int	dead_end;
extern	int x_cmdlist();



int	x_andor(node, input, output)
register ANDORPTR node; register int input, output;
BEGIN
    register int	a, d = dead_end;
    
PRINT("X_ANDOR\n");
    dead_end = FALSE;		/*  Suspend possibly impending death  */
    /*				/*  until after execution of the      */
    /*				/*  right member of the andor.        */
    
    SWITCH node->andortype IN
            
        CASE ANDTYPE:
            a = x_cmdlist(node->andorleft,  input, output);
            dead_end = d;
            IF a != 0 THEN
                IF dead_end THEN exit(a);
                ELSE return(a);
                FI
            ELSE
                return(x_cmdlist(node->andorright, input, output) );
            FI
            
        CASE ORTYPE:
            a = x_cmdlist(node->andorleft,  input, output);
            dead_end = d;
            IF a == 0 THEN 
                IF dead_end THEN exit(a);
                ELSE return(a);
                FI
            ELSE
                return(x_cmdlist(node->andorright, input, output) );
            FI
            
    ENDSW
END
