/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	=============================================
*
*	== strutil.c ==		 string manipulations
*
*	=============================================
*
*	Edit 003	1983-06-27
*
*	Modified by P-A Svensson
*	Dataindustrier AB T{by
*	Apr 1984
*/

#include "defs.h"

extern	char	*getstack();
extern	char	*blnkvar;

strless(s,t)		/*  String s < string t ?  */
register char *s, *t;	/*  (alphabetically, or, if both strings digital,  */
BEGIN			/*   numerically)         			   */
    register int	slen, tlen;
    
    IF  is_digit(*s) ANDF is_digit(*t) 
        ANDF (slen = strlen(s)) != (tlen = strlen(t))
    THEN
        return(slen<tlen);
    ELSE
	return(strcomp(s,t));
    FI
END



strcomp(s,t)
register char *s,*t;
BEGIN
        FOR ; *s == *t; s++, t++ DO
            IF *s == C_NUL THEN
                return(FALSE);
            FI
        OD
        return(*s < *t);
END

strequ(s,t)	/*  String s = string t ?  */
register char *s,*t;
BEGIN
    WHILE *s==*t++ DO
	IF *s++=='\0' THEN
	    return(TRUE);
	FI
    OD
    return(FALSE);
END


char *strcpy(s,t)		/* copy string t to string s */
register char *s, *t;
BEGIN
    WHILE *s++ = *t++ DO OD
    return(--s);
END

int instr(s,c)		/* position of first char c in string s */
register char *s,c;
BEGIN
    register int i=0;
    WHILE *s != C_NUL DO
        IF *s++ == c THEN return(i) FI
        i++;
    OD
    return(-1);
END

char *leftstr(s,n)		/*  chars 0 thru n of string s  */
register char *s; int n;
BEGIN
    register char *r,*t;
    register int i;
    r = t = getstack(n+2);
    FOR i=0;i<=n;i++ DO
        *t++ = *s++;
    OD
    *t=C_NUL;
    return(r);
END



char *rightstr(s,n)		/*  tail of s, from char no. n  */
register char *s; int n;
BEGIN
    register char *t;
    t = getstack(strlen(s+n)+2);
    strcpy(t,s+n);
    return(t);
END


strlen(s)			/*  length of string s  */
register char *s;
BEGIN
    register char *p = s;
    
    IF *p THEN WHILE *++p DO OD FI
    return(p-s);
END




char *conc3(s,t,u)			/*  concatenate 3 strings  */
register char *s, *t, *u;
BEGIN
    register char *r,*rr;
    
    r = rr = getstack(strlen(s)+strlen(t)+strlen(u)+1);
    WHILE *r++ = *s++ DO OD  --r;
    WHILE *r++ = *t++ DO OD  --r;
    WHILE *r++ = *u++ DO OD
    return(rr);
END

char *conc2(s,t)			/*  concatenate 2 strings  */
register char *s, *t;
BEGIN
    register char *r,*rr;
    
    r = rr = getstack(strlen(s)+strlen(t)+1);
    WHILE *r++ = *s++ DO OD  --r;
    WHILE *r++ = *t++ DO OD
    return(rr);
END


char *itoa(n)			/*  convert from integer to string  */
register int n;
BEGIN
    register int sign;
    register char *s;
    static char buf[10];
    
    *(s = &(buf[9]) ) = C_NUL;
    IF (sign = n) <0 THEN n = -n FI
    REPEAT
        *(--s) = n % 10 + '0';
    UNTIL (n /= 10) <= 0 DONE
    IF sign < 0 THEN *(--s) = '-' FI
    return(s);
END

char *itoo(n)			/*  convert from integer to octal  */
register int n;
BEGIN
    register int sign;
    register char *s;
    static char buf[10];
    
    *(s = &(buf[9]) ) = C_NUL;
    IF (sign = n) <0 THEN n = -n FI
    REPEAT
        *(--s) = n % 8 + '0';
    UNTIL (n /= 8) <= 0 DONE
    IF sign < 0 THEN *(--s) = '-' FI
    return(s);
END

int atoi(s)			/*  convert from string to integer  */
register char s[];
BEGIN
    register int i, n;
    register char *blank = blnkvar;
    
    FOR i=0; instr(blank,s[i]) >= 0; i++ DO OD
    FOR n=0; s[i] >= '0' ANDF s[i] <= '9'; i++ DO
        n = 10 * n + s[i] - '0';
    OD
    IF s[i] THEN return(-1);
    ELSE          return(n);
    FI
END

int otoi(s)			/*  convert octal string to integer  */
register char s[];
BEGIN
    register int i, n;
    register char *blank = blnkvar;
    
    FOR i=0; instr(blank,s[i]) >= 0; i++ DO OD
    FOR n=0; s[i] >= '0' ANDF s[i] <= '7'; i++ DO
        n = 8 * n + s[i] - '0';
    OD
    IF s[i] THEN return(-1);
    ELSE          return(n);
    FI
END


char **wltostar(wlist)		/*  convert a linked list of WORDS into   */
register WORDPTR wlist;		/*  an array of strings  */
BEGIN
    register int	count = 0;
    register char	**array, **aptr;
    register WORDPTR	wptr;
    
    FOR wptr = wlist; wptr; wptr = wptr->wordnxt DO
        ++count;
    OD
    aptr = array = (char **)getstack((count+1) * sizeof(char *));
    FOR wptr = wlist; wptr; wptr = wptr->wordnxt DO
        *aptr++ = wptr->wordtxt;
    OD
    *aptr = NIL;
    return(array);
END

char	*unquote(s)
register char	*s;
BEGIN
	register char *base,*src,*dst;

	IF !s THEN return(""); FI
	base = dst = src = rightstr(s,0);
	WHILE *src DO
		IF *src != C_DQUOT ANDF *src != C_SQUOT THEN
			*dst++ = *src & UNQUOTE;
		FI
		src++;
	OD
	*dst = *src;
    return(base);
END
