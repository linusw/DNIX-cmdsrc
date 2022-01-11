/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	=====================================================
*
*	== argcrunch.c ==	Do all kinds of substitutions
*
*	=====================================================
*
*	Edit 001	1983-09-06
*
*	Modified by P-A Svensson
*	Dataindustrier AB T{by
*	Apr 1984
*/

#include "defs.h"

extern	WORDPTR	expand();
extern	char	*rightstr();
extern	char	*blnkvar, *pathvar;
extern	char	*varsub();
extern	char	*cmdsub();
extern	char	*getstack();
extern	char	*ldpchar;
extern		strcpy();
static WORDPTR wildsub();

WORDPTR argcrunch(cmd)
WORDPTR cmd;
BEGIN
    register WORDPTR argptr, newarg, arglist, arglast;
    register char  *p, *s, *blankchar = blnkvar;
PRINT("ARGCRUNCH\n");
    
/*-----------------------------------------*/
/* Shell variable and command substitution */
/*-----------------------------------------*/

PRINT("\tVARIABLE & COMMAND SUBSTITUTION\n");
    arglist = (WORDPTR)NIL;

    FOR argptr = cmd; argptr; argptr = argptr->wordnxt DO
        IF *(p = (cmdsub(varsub(argptr->wordtxt)))) THEN	
            newarg = (WORDPTR)getstack(WORDSIZ);
            newarg->wordtxt = p;
            IF arglist THEN
                arglast = arglast->wordnxt = newarg;
            ELSE
                arglist = arglast = newarg;
            FI
        FI
    OD
    IF arglist THEN arglast->wordnxt = (WORDPTR)NIL FI

PRINT("\tSPLIT ARGUMENTS\n");
/*-----------------------------*/
/* If an argument contains a   */
/* blank, split it in two, and */
/* insert a new node in list   */
/*-----------------------------*/

    FOR argptr = arglist; argptr; argptr = argptr->wordnxt DO
        WHILE instr(blankchar, *(argptr->wordtxt) ) >= 0 DO		/*  Skip leading blanks  */
            ++(argptr->wordtxt);
        OD
        
        FOR p = argptr->wordtxt;	/*  Search non-leading blank  */
        *p ANDF (instr(blankchar, *p) < 0);
	DO
		IF *p == C_DQUOT THEN
			WHILE *(++p) ANDF *p != C_DQUOT DO
				/* No pattern expansion within quotes */
				*p |= QUOTE;
			OD
			IF !(*p) THEN p--; FI; /* In case no ending quote */
		FI
		p++;
	OD
        IF *p THEN			/*  If found then split  */
            newarg = (WORDPTR)getstack(WORDSIZ);
            newarg->wordnxt = argptr->wordnxt;
            newarg->wordtxt = p+1;
            
            argptr->wordnxt = newarg;
            *p = C_NUL;
        FI
    OD
/*--------------------------*/
/* Wildcard substitution   */
/*-------------------------*/

PRINT("\tWILDCARD SUBSTITUTION\n");
    arglist = wildsub(arglist);	/*  Substitute for wildcards    */

/*-------------------------*/
/*  Unquote all arguments  */
/*-------------------------*/

PRINT("\tUNQUOTE ARGUMENTS\n");
    FOR argptr = arglist; argptr; argptr = argptr->wordnxt DO
        FOR p = argptr->wordtxt, s = p; *p; ++p DO
		IF *p != C_SQUOT ANDF *p != C_DQUOT THEN
			*s++ = *p & UNQUOTE;
		FI
        OD
   	*s = *p;
    OD

PRINT("END\n");
    return(arglist);
END




    /*---------------------------------------------*/
    /*  Substitute for wildcards, expanding every  */
    /*  node into one node for every match found   */
    /*---------------------------------------------*/
static WORDPTR wildsub(arglist)
WORDPTR arglist;
BEGIN
    register  WORDPTR sublist, *ptradr, w0, w1;
    register char c,*p,*s;
    register int noswap;
    WORDPTR sublast;
    
    sublist = (WORDPTR)NIL;
    FOR ptradr = &arglist; *ptradr; DO
	p = (*ptradr)->wordtxt;
	s = p;
	WHILE c = *s++ DO
		IF (c == WC_LPAR) ORF (c == WC_CHAR) ORF (c == WC_STR) THEN
			break;
		FI
	OD
	IF c == '\000' THEN
		sublist = (WORDPTR)NIL;
	ELIF *p == C_PATH THEN
            sublist=expand("/",rightstr(p,1),&sublast);
	ELSE
	    sublist = expand(".", p,&sublast);
        FI
        IF sublist THEN
		REPEAT
		/* Bubbles!!! */
			noswap = TRUE;
			w0 = sublist;
			WHILE w0 != sublast DO
				w1 = w0->wordnxt;
				IF !strcomp(w0->wordtxt,w1->wordtxt) THEN
					noswap = FALSE;
					p = w0->wordtxt;
					w0->wordtxt = w1->wordtxt;
					w1->wordtxt = p;
				FI
				w0 = w1;
			OD
		UNTIL noswap DONE
            sublast->wordnxt = (*ptradr)->wordnxt;
            *ptradr = sublist;
            ptradr = &(sublast->wordnxt);
        ELSE
            ptradr = &( (*ptradr)->wordnxt);
        FI
    OD
    return(arglist);
END
