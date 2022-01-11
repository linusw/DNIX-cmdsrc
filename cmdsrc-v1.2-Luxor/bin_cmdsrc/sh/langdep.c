/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	================================================
*
*	== langdep.c ==		Language depentent stuff
*
*	================================================
*
*	Edit 001	1983-08-25
*/

#include "defs.h"

char engchar[] = { '{', '}', '\\', '[', ']', '`', '\0' };
char swechar[] = { ':', ':', '%',  ':', ':', '#', '\0' };

char *ldpchar = engchar;


/*--------------------------------------------------------------*/
/*  In addition to the reserved words, some special characters  */
/*  differ between UNIX and DIAB syntax.                        */
/*  They are - from left to right - :                           */
/*  - beginning and end  of substitution, as in ${name=word}    */
/*  - the quote character                                       */
/*  - beginning and end of a set of characters in a gen. expr.  */
/*  - the command substitution character                        */
/*--------------------------------------------------------------*/
