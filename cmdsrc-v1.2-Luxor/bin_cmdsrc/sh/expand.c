/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	===================================================================
*
*	== expand.c ==		return ptr to list of file names 
*				that match template filename.
*
*		Parameters:
*				dirname		where to look
*				filename	what to look for
*				last		adress of ptr to end of 
*						returned list
*
*	===================================================================
*
*	Edit 001	1983-06-20
*/

#include "defs.h"
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>

extern	char	*ldpchar;
extern	char	*getstack();
extern	char	*unquote();
extern	WORDPTR	dirlook();
extern	int	instr();
extern	char	*leftstr();
extern	char	*rightstr();

WORDPTR expand(dirname,filename,last)
char *dirname;
register char *filename;
register WORDPTR *last;
BEGIN
    register int p;
    register char *s;
    register char c;
    register WORDPTR first = (WORDPTR)NIL;
    register WORDPTR dirlist,sublist;
    WORDPTR dirlast,sublast;
    struct stat statblk;
    
PRINT("EXPAND\n");
    s = filename;
    IF s == NULL THEN return((WORDPTR)NIL); FI
/*
    WHILE c = *s++ DO
	IF (c == WC_LPAR) ORF (c == WC_CHAR) ORF (c == WC_STR) THEN
		break;
	FI
    OD
    IF c == '\000' THEN
		return((WORDPTR)NIL);
    FI
 */
    IF (p=instr(filename,C_PATH)) >= 0 THEN
        dirlist = dirlook(dirname,leftstr(filename,p-1),&dirlast,TRUE);
        WHILE dirlist DO
	    sublist = expand(dirlist->wordtxt,rightstr(filename,p+1),&sublast);
            IF sublist THEN
                IF !first THEN
                    first = sublist;
                ELSE
                    (*last)->wordnxt = sublist;
                FI
                *last = sublast;
            FI
            dirlist = dirlist->wordnxt;
        OD
PRINT("END EXPAND\n");
        return(first);
    ELSE
PRINT("END EXPAND\n");
        return( dirlook(dirname,filename,last,FALSE) );
    FI
END
