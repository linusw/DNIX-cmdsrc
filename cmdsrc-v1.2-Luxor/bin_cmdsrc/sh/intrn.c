/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	====================================
*
*	== intern.c ==	Internal shell commands
*
*	====================================
*
*	Edit 001	1983-07-22
*/

#include "defs.h"

struct atoitab intrncmd[] =
BEGIN
    ":",	I_NOP,
    "#",	I_NOP,
    ".",	I_PARSEX,
    "break",	I_BREAK,
    "continue",	I_CONT,
    "cd",	I_CD,
    "eval",	I_EVAL,
    "exec",	I_EXEC,
    "exit",	I_EXIT,
    "export",	I_EXPORT,
    "login",	I_LOGIN,
    "newgrp",	I_NEWGRP,
    "read",	I_READ,
    "readonly",	I_RDONLY,
    "set",	I_SET,
    "shift",	I_SHIFT,
    "times",	I_TIMES,
    "trap",	I_TRAP,
    "umask",	I_UMASK,
    "wait",	I_WAIT,
    "english",	I_ENG,
    "swedish",	I_SWE
END;

int nintrn = sizeof(intrncmd)/sizeof(struct atoitab);		/*  Number of internal commands  */

intrncode(cmd)		/*  If cmd is internal cmd, return code  */
register char *cmd;	/*  else return 0                         */
BEGIN
    register int i;
    
    FOR i=0; i<nintrn; ++i DO
        IF strequ(cmd, intrncmd[i].tabstr) THEN
            return(intrncmd[i].tabcode);
        FI
    OD
    return(0);
END
