/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	======================================
*
*	== rsrvword.c ==	reserved words
*
*	======================================
*
*	Edit 001	1983-05-18
*/

#include "defs.h"

extern int	strequ();

struct atoitab sweword[] =
BEGIN
    "begin",	R_BGN,
    "end",	R_END,
    "for",	R_FOR,
    "while",	R_WHI,
    "until",	R_UNT,
    "done",	R_DONE,
    "do",	R_DO,
    "case",	R_CAS,
    "in",	R_IN,
    "esac",	R_ESAC,
    "if",	R_IF,
    "then",	R_THEN,
    "else",	R_ELSE,
    "elif",	R_ELIF,
    "fi",	R_FI,
    "!!",	R_ORF,
    "&&",	R_ANDF,
    "<<",	R_HDOC,
    ">>",	R_APP,
    "<&-",	R_CLINP,
    ">&-",	R_CLOUT,
    "<&",	R_INDUP,
    ">&",	R_OUTDUP,
    "(",	R_LGR,
    ")",	R_RGR,
    "!",	R_PIPE,
    "&",	R_BACK,
    ";;",	R_ENDCAS,
    ";",	R_SEP,
    "<",	R_INP,
    ">",	R_OUT,
    "\n",	R_NL,
    "\377",	R_EOF,
END;

struct atoitab engword[] =
BEGIN
    "{",	R_BGN,
    "}",	R_END,
    "for",	R_FOR,
    "while",	R_WHI,
    "until",	R_UNT,
    "done",	R_DONE,
    "do",	R_DO,
    "case",	R_CAS,
    "in",	R_IN,
    "esac",	R_ESAC,
    "if",	R_IF,
    "then",	R_THEN,
    "else",	R_ELSE,
    "elif",	R_ELIF,
    "fi",	R_FI,
    "||",	R_ORF,
    "&&",	R_ANDF,
    "<<",	R_HDOC,
    ">>",	R_APP,
    "<&-",	R_CLINP,
    ">&-",	R_CLOUT,
    "<&",	R_INDUP,
    ">&",	R_OUTDUP,
    "(",	R_LGR,
    ")",	R_RGR,
    "|",	R_PIPE,
    "&",	R_BACK,
    ";;",	R_ENDCAS,
    ";",	R_SEP,
    "<",	R_INP,
    ">",	R_OUT,
    "\n",	R_NL,
    "\377",	R_EOF,
END;

struct atoitab *rsrvword = engword;


int	nrsrv = sizeof(engword)/sizeof(struct atoitab);	/*  Number of reserved words  */


/*---------------------------------------------------------------*/
/*  These characters cause termination of a word unless quoted   */
/*  ( And so do the characters in $IFS, the value of BLNKVAR )   */
/*---------------------------------------------------------------*/

char swespch[] = { '!', '&', ';', '<', '>', '(', ')', '\n', '\0' };
char engspch[] = { '|', '&', ';', '<', '>', '(', ')', '\n', '\0' };

char *specchar = engspch;



char	*getrsrv(r)		/*  Return reserved word that has R-code r  */
register int	r;
BEGIN
    register int i;
    FOR i=0 ; i<nrsrv ; ++i DO
        IF (rsrvword+i)->tabcode == r THEN return((rsrvword+i)->tabstr) FI
    OD
    return("");
END
