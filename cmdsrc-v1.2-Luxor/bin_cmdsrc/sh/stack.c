/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	=====================================
*
*	== stack.c ==	General purpose stack
*
*	=====================================
*
*	Edit 001	1983-07-22
*/

#include "defs.h"

#define  SEGMSIZ 2048

char *stackptr;
char **cursegm;
char **stackbot;


stackinit()
BEGIN
    stackbot = cursegm = (char **)calloc(SEGMSIZ,sizeof(char) );
    stackptr = (char *)(stackbot + 1);					/*  Leave room for link  */
    *stackbot = NIL;							/*  Initialize link to NIL  */
END


clearstack()								/*  Free all segments but the first  */
BEGIN
    register char **p, **q;
    
    p = (char **)*stackbot;
    
    WHILE p DO
        q =(char **)*p;
        free(p);
        p = q;
    OD
    
    cursegm = stackbot;
    *stackbot = NIL;
    stackptr = (char *)(stackbot +1);
END



char *getstack(n)
register int n;
BEGIN
    register char *p;

    stackptr = (char *)( ( (int)stackptr + 1)&(~1) );			/*  align  */
    
    IF SEGMSIZ - (stackptr - (char *)cursegm) >= n THEN			/*  n bytes available ?  */
        p = stackptr;							/*  yes  */
        stackptr = stackptr + n;
        return(p);
    ELSE								/*  no - start new segment  */
        cursegm = *( (char ***)cursegm ) =
        (char **)calloc(SEGMSIZ * (1+ n/(SEGMSIZ-sizeof(char *) ) ), sizeof(char) );
        *cursegm = NIL;
        stackptr = (char *)(cursegm +1) + n;
        return( (char *)(cursegm+1) );
    FI
END
