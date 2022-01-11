/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	===========================
*
*	== flag.c ==	Shell flags
*
*	===========================
*
*	Edit 001	1983-07-25
*/

#include "defs.h"
int	flags;
char	flagname[] = {'e'    ,'i'    ,'k'    ,'n'    ,'r'    ,'s'    ,'t'    ,'u'    ,'v'    ,'x'    ,'c'    ,C_NUL };
int     flagbit [] = { E_FLAG, I_FLAG, K_FLAG, N_FLAG, R_FLAG, S_FLAG, T_FLAG, U_FLAG, V_FLAG, X_FLAG, C_FLAG, 0 };


int	setflag(c)
register char	c;
BEGIN
    register int n;

    IF (n = instr(flagname,c) ) >= 0 THEN
        flags |= flagbit[n];
        setflvar();
        return(TRUE);
    ELSE
        return(FALSE);
    FI
END


resetflag(c)
register char c;
BEGIN
    register int n;

    IF (n =instr(flagname,c) ) >= 0 THEN
        flags &= ~flagbit[n];
        setflvar();
        return(TRUE);
    ELSE
        return(FALSE);
    FI
END

setflvar()
BEGIN
    register char s[20], *t;
    register int  i;
    
    t = s;
    FOR i=0; flagbit[i]; ++i DO
        IF flags & flagbit[i] THEN
            *t++ = flagname[i];
        FI
    OD
    *t = C_NUL;
    setvar(FLAGVAR,s);
END
