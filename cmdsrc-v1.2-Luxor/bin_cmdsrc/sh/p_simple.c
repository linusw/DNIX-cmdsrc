/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	============================================
*
*	== p_simple.c ==	Parse simple command
*
*	============================================
*
*	Edit 008	1983-08-04
*/

#include "defs.h"
#include	"cmdbuf.h"

extern	char	*p_word();
extern	char	*getstack();
extern	IOPTR	p_io();
extern	char	*leftstr();
extern	char	*rightstr();
extern	int	flags;


NODPTR p_simple()
BEGIN
    register SIMPPTR sp = (SIMPPTR)NIL;
    ASGNPTR *lastasgn, makeasgn();
    WORDPTR *lastarg, makearg();
    register IOPTR   *lastio;
    register char *w;
    register int i, fd, equpos;
    
PRINT("P_SIMPLE\n");
    sp           = (SIMPPTR)getstack(SIMPSIZ);
    sp->simptype = SIMPTYPE;
    sp->simpnxt  = (NODPTR)NIL;
    sp->simpback = FALSE;
    sp->simpasgn = (ASGNPTR)NIL;
    sp->simpio   = (IOPTR)NIL;
    sp->simpcmd  = (WORDPTR)NIL;
    
    lastio   = &(sp->simpio);
    lastarg  = &(sp->simpcmd);
    lastasgn = &(sp->simpasgn);
    
    
    REPEAT
        w = p_word(TRUE);						/*  Try to read a word  */
        fd = -1;							/*  Assume no file descriptor  */
        
        IF  (  !(*w)							/*  If there was no word  */
                ORF
                (   (fd = atoi(w) ) >= FDMIN				/*  or the word was a valid fd  */
                    ANDF
                    fd <= FDMAX 
                )
            )
            ANDF
            ( *lastio = p_io(fd) )					/*  Then try to read an io  */
        THEN
            lastio = &( (*lastio)->ionxt );
        ELIF
            ( equpos = instr(w, C_EQU) ) > 0				/*  If w contains '='  */
            ANDF
            is_varname(leftstr(w,equpos-1))				/*  preceded by a variable name  */
            ANDF
            (   sp->simpcmd == (WORDPTR)NIL				/*  and no command found yet  */
                ORF
                flags & K_FLAG						/*  or -k flag set          */
            )
            ANDF						        /*  Then create assignment node  */
            ( *lastasgn = makeasgn(w, equpos) )
        THEN
            lastasgn  = &( (*lastasgn)->asgnnxt );
        ELIF *w THEN							/*  Otherwise it's an argument  */
            *lastarg = makearg(w);
            lastarg  = &( (*lastarg)->wordnxt );
        ELSE
            return( (NODPTR)sp );
        FI
    UNTIL DOOMSDAY
    
END


WORDPTR makearg(w)		/*  Make an argument node  */
register char *w;
BEGIN
    register WORDPTR wp;
    
    wp          = (WORDPTR)getstack(WORDSIZ);
    wp->wordtxt = w;
    wp->wordnxt = (WORDPTR)NIL;
    return(wp);
END


ASGNPTR makeasgn(w, equpos)	/*  Make an assignment node  */
register char *w;
register int equpos;

BEGIN
    register ASGNPTR	ap;
    
    ap           = (ASGNPTR)getstack(ASGNSIZ);
    ap->asgnnxt  = (ASGNPTR)NIL;
    ap->asgnname = leftstr (w, equpos-1);
    ap->asgnval  = rightstr(w, equpos+1);
    return(ap);
END
