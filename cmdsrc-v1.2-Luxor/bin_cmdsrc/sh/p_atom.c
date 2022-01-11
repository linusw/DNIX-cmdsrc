/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	======================================================
*
*	== p_atom.c ==	parse a word, a reserved or an integer
*
*	======================================================
*
*	Edit 009	1983-07-14
*
*	Modified by P-A Svensson
*	Dataindustrier AB T{by
*	Apr 1984
*/


#include	"defs.h"
#include	"cmdbuf.h"


extern	char		readchar();
extern	char		nextchar();
extern	char		*cmdcop();
extern			skipblen();
extern	int		nrsrv;
extern	char		*specchar;
extern	struct atoitab	*rsrvword;
extern	char		*secvar, *blnkvar;
extern	char		*ldpchar;
char			maxlc = 'z';
char			maxuc = 'Z';

p_rsrv(sxlb)		/* Try to read a reserved word  */
register int sxlb;	/* If sxlb then search across line boundaries  */
BEGIN
    register int	start, i, code = 0;
    register char	*rp, *prompt = secvar;
    
PRINT("P_RSRV\n");
    IF sxlb THEN				      /*  Skip blanks  */
        skipblanks(prompt);
    ELSE
        skipblen(prompt);
    FI
    start = cmdpos();
    FOR i=0 ; i<= nrsrv-1 ANDF code == 0 ; ++i DO
        FOR rp = (rsrvword+i)->tabstr ; *rp ; ++rp DO
            IF *rp != readchar(prompt) THEN
                setcmdpos(start);
                BREAK;
            FI
        OD
        IF *rp == C_NUL THEN code = (rsrvword+i)->tabcode FI
    OD

/*------------------------------------------------------*/
/* Those reserved words that don't begin with a special */
/* character must be terminated with a delimiter.       */
/*------------------------------------------------------*/

    IF code ANDF instr(specchar, *(rsrvword+i-1)->tabstr) < 0
       ANDF !delim(nextchar("") )
    THEN
        setcmdpos(start);
        return(0);
    FI
    return(code);
END


char *p_word(copy)		/*  Try to read a word  */
register int copy;
BEGIN
    register int start;
    register char c,ch;
    
PRINT("P_WORD\n");
    skipblen(secvar);
    start = cmdpos();
    IF instr(specchar,(c = nextchar(secvar)) ) >= 0 ORF c == C_EOF THEN 
	return("");
    FI
    REPEAT
	/* Pass quouted strings */
	c = readchar(secvar);
	IF c == C_SQUOT ORF c == C_DQUOT ORF c == C_CMDSUB THEN
		WHILE (ch = readchar(secvar)) != c DO 
			IF ch == C_EOF THEN
				fatalerr(ER_EOF);
			FI
		OD
	FI
    UNTIL delim(c) DONE;
    IF cmdpos() - start > 1 THEN setcmdpos(cmdpos()-1) FI
    return(cmdcop(start,cmdpos(),copy));
END

int p_int(min,max)	/*  Try to read a positive int within limits  */
register int min,max;
BEGIN
    register int	a,start;
    register char	buf[6],*p;

    p = buf;
    skipblen("");
    start = cmdpos();
    WHILE is_digit(*p++ = readchar("")) ANDF  p - buf <6 DO OD
    setcmdpos(cmdpos()-1);
    *(--p) = C_NUL;
    IF p == buf ORF p -buf >5
        ORF (a = atoi(buf) ) < min ORF a > max
    THEN setcmdpos(start) ;return(-1);
    ELSE return(a);
    FI
END

int delim(c)
register char c;
BEGIN
    IF  c == C_EOF
        ORF instr(specchar,c) >= 0
	/* Space or tab instead of blnkvar */
        ORF instr(" \t",c) >= 0
    THEN
        return(TRUE);
    ELSE
        return(FALSE);
    FI
END


int is_letter(c)
register char c;

BEGIN
    return((c >= 'A' ANDF c <= maxuc) ORF (c >= 'a' ANDF c <= maxlc));
END


int is_digit(c)
register char c;
BEGIN
    IF c>='0' ANDF c<='9'
    THEN return(TRUE);
    ELSE return(FALSE);
    FI
END
