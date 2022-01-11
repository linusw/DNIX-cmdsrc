/*
 *
 *	DNIX SHELL
 *
 *	P-A Svensson
 *	Dataindustrier AB T{by
 *
 *	====================================================
 *
 *	==	upd_vars.c ==	often used shell variables
 *
 *	====================================================
 *
 *	1984-04-19
 */

#include	"defs.h"

extern	char	*getvar();

/*	Internal fast access shell variables:				*/
char	*primvar	= NULL;
char	*secvar		= NULL;
char	*blnkvar	= NULL;
char	*homevar	= NULL;
char	*pathvar	= NULL;

/*	Routine for updating the internal shell variables that are	*/
/*	maintained as strings for the purpouse of fast access.		*/
upd_vars()
BEGIN
	primvar	= getvar(PRIMVAR);
	secvar	= getvar(SECVAR);
	blnkvar	= getvar(BLNKVAR);
	homevar	= getvar(HOMEVAR);
	pathvar	= getvar(PATHVAR);
END
