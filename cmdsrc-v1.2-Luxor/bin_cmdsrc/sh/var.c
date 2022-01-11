
/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	=======================================
*
*	== var.c ==	shell variable handling
*
*	=======================================
*
*	Edit 001	1983-06-29
*
*	Modified by P-A Svensson
*	Dataindustrier AB T{by
*	Apr 1984
*
*	Edit 002	1984-12-11
*
*	Modified by Magnus Hedner
*	Dataindustrier AB T{by
*	Dec 1984
*/

#include	"defs.h"

extern	int	strequ();
extern	char	*getnode();
extern	int	strlen();
extern char	*strcpy();
extern	char	*leftstr();
extern	char	*rightstr();
extern	char	*conc3();
extern	int	is_digit();
extern	int	is_letter();
extern	char	*unquote();
extern	char	*getstack();
extern	char	*cmdsub();
extern	int	interactive;
extern	jmp_buf	mainenv;
extern	char	*ldpchar;
extern	char	**environ;
extern	int	upd_vars();

char		*varsub();
char		**setenv();
VARPTR		varlist;
char		empty[] = "";	/*	Empty string			*/

varinit()		/*  initialize variable list  */
BEGIN
    varlist = (VARPTR)calloc(VARSIZ,sizeof(char));
    varlist->varnxt    = (VARPTR)NIL;
    varlist->varnam    = "";				/*  Dummy  */
    varlist->varval    = "";
    varlist->varexp    = TRUE;
    varlist->varrdonly = FALSE;
    
    IF geteuid() == 0 THEN
	    setvar(PRIMVAR, PRIMSINI);			/* PS1 - su */
    ELSE
	    setvar(PRIMVAR, PRIMINI);			/*  PS1  */
    FI
    setvar(SECVAR,  SECINI);				/*  PS2  */
    setvar(BAKVAR,  BAKINI);				/*  "0"  */
    setvar(BLNKVAR, BLNKINI);				/*  " \t\n"  */
    markvar(BLNKVAR, EXPORT);
    setvar(PARVAR,  PARINI);				/*  "0"  */
    setvar(LASTVAR, LASTINI);				/*  "0"  */
	setvar(PATHVAR, PATHINI);
	markvar(PATHVAR, EXPORT);
	setvar(LANGVAR, LANGINI);
	markvar(LANGVAR, EXPORT);
	upd_vars();		/*	Initialize pointers	*/
END

setvar(name,value)		/*  variable name := value  */
register char	*name, *value;
BEGIN
    register VARPTR	p, q, lastp;
    
PRINT("SETVAR\n");
    p = varlist;
    WHILE p ANDF strless(p->varnam,name) DO
        lastp = p;
        p = p->varnxt;
    OD

    IF p ANDF strequ(p->varnam,name) THEN
        IF p->varrdonly THEN
            errbrk(name);errline(ER_RDONLY);
            return;
        FI
    ELSE    
        q         = lastp->varnxt = (VARPTR)calloc(VARSIZ,sizeof(char));
        q->varnxt = p;
        q->varnam = (char *)calloc(strlen(name)+1,sizeof(char));
        strcpy(q->varnam,name);
        q->varval = (char *)calloc(1,sizeof(char));
        *q->varval = C_NUL;
        q->varexp = FALSE;
        q->varrdonly = FALSE;
        p = q;
    FI
    IF strlen(p->varval) < strlen(value) THEN
        free(p->varval);
        p->varval = (char *)calloc(strlen(value)+1,sizeof(char) );
    FI
    strcpy(p->varval,value);
	IF p->varexp THEN
		setenv();
	FI
	upd_vars();	/*	Update shell internal variable pointers	*/
END

char *getvar(name)		/*  return pointer to value of variable  */
register char *name;
BEGIN
    register VARPTR vp, pp;
    register char *s, *value, c, **pt, **pvalue;
    register int  totlen, i;

    IF *name == C_LVARX THEN
        FOR value = ++name; *value != C_RVARX; DO ++value OD
        *value =C_NUL;
        FOR value = name; *value ANDF instr("-=?+", *value) < 0; DO ++value OD
        c = *value; *value++ = C_NUL;
        IF *(s = getvar(name)) THEN
            IF c == '+' THEN return( cmdsub(varsub(value)) );
            ELSE return(s) FI
        ELSE
            IF c != '+' THEN value = cmdsub(varsub(value)) FI
            SWITCH c IN
                CASE '=':
                    setvar(name,value);
                CASE '-':
                    return(value);
                CASE '?':
                    prbrk(name);prbrk(": ");
                    IF *value THEN
                        prline(value);
                    ELSE
                        prline(ER_PNSET);
                    FI
                    IF interactive THEN
                        longjmp(mainenv, 1);
                    ELSE
                        exit(0);
                    FI
                CASE '+':
                CASE C_NUL:
                    return("");
            ENDSW   
        FI
    ELIF strequ(name, STARVAR) ORF strequ(name, ALLVAR) THEN
        FOR vp = varlist; vp ANDF *(vp->varnam)<'1'; vp = vp->varnxt DO OD
        FOR pp=vp,totlen = 0; vp ANDF *(vp->varnam)<='9'; vp = vp->varnxt DO
            totlen = totlen + strlen(vp->varval)+1
        OD
	IF totlen == 0 THEN return(""); FI
        s = value = getstack(totlen); *s = C_NUL;
        FOR vp = pp; vp ANDF *(vp->varnam)<='9'; vp = vp->varnxt DO
		s = strcpy(s,vp->varval);
		s = strcpy(s," ");
        OD
	*(--s) = C_NUL;
        return(value);
/*    ELIF strequ(name, ALLVAR) THEN
	FOR vp = varlist; vp ANDF *(vp->varnam)<'1'; vp = vp->varnxt DO OD
	FOR i=1,pp=vp,totlen=0; vp ANDF *(vp->varnam)<='9'; vp = vp->varnxt DO
	    i++;
	    totlen += strlen(vp->varval) + 1;
	OD
	IF totlen == 0 THEN return(&empty); FI
	s = getstack(totlen);
	pt = pvalue = (char **)getstack(i * sizeof(char *));
	FOR vp = pp; vp ANDF *(vp->varnam)<='9'; vp = vp->varnxt DO
	    *pt++ = s;
	    *s = C_NUL;
	    s = strcpy(s, vp->varval);
	    *s++ = C_NUL;
	OD
	*pt = NULL;
	return(pvalue);	*/
    ELIF *name == C_NUL THEN return("$");
    ELSE
        vp = varlist;
        WHILE vp != (VARPTR)NIL ANDF !strequ(vp->varnam,name) DO
            vp = vp->varnxt;
        OD
        IF !vp THEN
            return("");
        ELSE
            return(vp->varval);
        FI
    FI
END

markvar(name, m)		/*  Mark variable 'name' as exportable  */
register char *name;		/*  or 'readonly'.                      */
BEGIN
    register VARPTR p;

PRINT("MARKVAR\n");
    p = varlist;				/*  Create if non-existent  */
    WHILE p ANDF strless(p->varnam,name) DO
        p = p->varnxt;
    OD
    IF !(p ANDF strequ(p->varnam,name) ) THEN
        setvar(name, getvar(name));
    FI
    p = varlist;
    WHILE p ANDF strless(p->varnam, name) DO
        p = p->varnxt;
    OD
    IF   m == EXPORT THEN p->varexp    = TRUE;
    ELIF m == RDONLY THEN p->varrdonly = TRUE;
    FI

END


char *varsub(s)			/*  Replace all variable names in s  */
register char	*s;		/*  with their current values        */
BEGIN
    register int i,p = 0;
    char *readvarname();
    register char *varname, *varval;

PRINT("VARSUB\n");
    WHILE (i = instr(s+p,C_VAR)) >=0 DO
        p += i;
        varname = readvarname(s+p+1);
        s = conc3(leftstr(s,p-1),varval = getvar(varname),rightstr(s,p+strlen(varname)+1));
        p = p + strlen(varval);
    OD
    return(s);
END

char	*readvarname(s)		/*  Read variable name pointed to by s  */
register char *s;
BEGIN
    register char *t = s;

    IF instr(SPECVAR,*t) >= 0 THEN
        return(leftstr(t,0));
    ELIF is_letter(*t) THEN
        WHILE is_letter(*t) ORF is_digit(*t) DO ++t OD
    ELIF is_digit(*t) THEN
        WHILE is_digit(*t) DO ++t OD
    ELIF *t == C_LVARX THEN 
        ++t;
        WHILE *t++ != C_RVARX DO OD
    FI
    return(leftstr(s,t-s-1));
END

WORDPTR posparlst()		/*  WORDlist of positional parameters  */
BEGIN
    register WORDPTR first = (WORDPTR)NIL, last;
    register VARPTR vp;
    
    FOR vp = varlist; vp ANDF *(vp->varnam) <  '1'; vp = vp->varnxt DO OD
    FOR             ; vp ANDF *(vp->varnam) <= '9'; vp = vp->varnxt DO
        IF !first THEN
            first = last = (WORDPTR)getstack(WORDSIZ);
        ELSE
            last = last->wordnxt = (WORDPTR)getstack(WORDSIZ);
        FI
        last->wordnxt = (WORDPTR)NIL;
        last->wordtxt = getstack(strlen(vp->varval)+1);
        strcpy(last->wordtxt,vp->varval);
    OD
    return(first);
END

clrpospar()		/*  Clear positional parameters  */
BEGIN
    register VARPTR vp, lastv;
    
    FOR vp = varlist; vp ANDF *(vp->varnam) < '1';lastv = vp, vp = vp->varnxt DO OD
    FOR ; vp ANDF *(vp->varnam) <= '9'; DO
        lastv->varnxt = vp->varnxt;
        free(vp->varnam); free(vp->varval); free(vp);
        vp = lastv->varnxt;
    OD
END

shift()				/*  $1 := $2 etc.  */
BEGIN
    register VARPTR vp, tmp;
    
    FOR vp = varlist; vp->varnxt ANDF *(vp->varnxt->varnam) < '1'; DO
	vp = vp->varnxt;
    OD
    IF vp->varnxt ANDF *(vp->varnxt->varnam) == '1' THEN
        tmp = vp->varnxt;
        vp = vp->varnxt = vp->varnxt->varnxt;
        free(tmp->varnam); free(tmp->varval); free(tmp);
        FOR ; vp ANDF *(vp->varnam) <= '9'; vp = vp->varnxt DO
            strcpy(vp->varnam,itoa(atoi(vp->varnam)-1));
        OD
	setvar(PARVAR, itoa(atoi(getvar(PARVAR)) - 1));
        return(0);
    ELSE
        errline(ER_CNSH);
        return (-1);
    FI
END

kleenex()			/*  Clear non-exportable variables  */
BEGIN
    register VARPTR p;
    register VARPTR *lastp;
    
PRINT("KLEENEX\n");
    lastp = &varlist;
    p     =  varlist;
    WHILE p DO
        IF p->varexp THEN
            lastp = &(p->varnxt);
        ELSE
            *lastp = p->varnxt;
        FI
        p = p->varnxt;
    OD
END


char **setenv()
BEGIN
    register VARPTR vp;
    register int count = 1;
    register char **e, **p;

PRINT("SETENV\n");
    /*	Free old environment						*/
    IF environ THEN
	register char **env = environ;
	WHILE *env DO
		free(*env);
		env++;
	OD
	free(environ);
    FI
    FOR vp = varlist; vp; vp = vp->varnxt DO
        IF vp->varexp ANDF *vp->varnam THEN ++count FI
    OD
    e = p = (char **)calloc(count, sizeof(char *));
    FOR vp = varlist; vp; vp = vp->varnxt DO
        IF vp->varexp ANDF *vp->varnam THEN
	    *p = (char *)calloc(strlen(vp->varnam)+strlen(vp->varval)+2, sizeof(char));
	    strcpy(*p, vp->varnam);
	    strcat(*p, "=");
	    strcat(*p, vp->varval);
	    p++;
        FI
    OD
    *p = NIL;
    environ = e;
    return(e);
END
