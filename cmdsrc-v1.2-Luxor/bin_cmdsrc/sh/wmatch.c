/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	====================================
*
*	== wmatch.c ==		wildcard match
*
*	====================================
*
*	Edit 001	1983-05-20
*/

#include "defs.h"

extern	char	*ldpchar;


/*-----------------------------------------------------------------*/
/*  This routine tries to match 'wild', a string that may contain  */
/*  wildcard characters ( *, ?, or [...] ), against the string     */
/*  'name', a filename or a case variable.                         */
/*-----------------------------------------------------------------*/

wmatch(name,wild,firstch)
register char *name,*wild;
register int firstch;
BEGIN
    register char nc,wc;
    
    nc = *name++; wc= *wild++;
    IF firstch ANDF nc == C_WDIR ANDF wc != nc THEN
	return(FALSE);
   FI
    IF wc == WC_LPAR THEN
        BEGIN
            int found = FALSE;
            char lc = (char)127;

            WHILE wc = *wild++ DO
                IF wc == WC_RPAR THEN
                    return(found ? wmatch(name,wild,FALSE) : FALSE);
                ELIF wc == WC_SEQ THEN
                    IF lc <= nc ANDF nc <= (*wild++) THEN found = TRUE FI
                ELSE
                    IF nc == (lc = wc) THEN found = TRUE FI
                FI
            OD
            return(FALSE);
        END
    ELIF wc == WC_CHAR THEN			/*  '?'  */
        return( nc != C_WDIR ANDF nc ? wmatch(name,wild,FALSE) : FALSE);
    ELIF wc == WC_STR THEN			/*  '*'  */
        IF *wild == C_NUL THEN return(TRUE);
        ELSE --name;
            WHILE *name DO
                IF wmatch(name++,wild,FALSE) THEN return(TRUE) FI
            OD
            return(FALSE);
        FI
    ELIF wc == C_NUL THEN            
        return(nc==C_NUL);
    ELSE
    return( (nc & UNQUOTE) == (wc & UNQUOTE) ? wmatch(name,wild,FALSE) : FALSE);
    FI
END
