/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	=======================
*
*	== sign.c ==	signals
*
*	=======================
*
*	Edit 001	1983-08-10
*/

#include	"defs.h"

extern	int	interactive;
extern	int	flags;
extern	jmp_buf	mainenv;

int	(*oldsig[NSIG])();
char	*trapcmd[NSIG];

trap0() { signal(0,trap0); trap(0);}
trap1() { signal(1,trap1); trap(1);}
trap2() { signal(2,trap2); trap(2);}
trap3() { signal(3,trap3); trap(3);}
trap4() { signal(4,trap4); trap(4);}
trap5() { signal(5,trap5); trap(5);}
trap6() { signal(6,trap6); trap(6);}
trap7() { signal(7,trap7); trap(7);}
trap8() { signal(8,trap8); trap(8);}
trap9() { signal(9,trap9); trap(9);}
trap10() { signal(10,trap10); trap(10);}
trap11() { signal(11,trap11); trap(11);}
trap12() { signal(12,trap12); trap(12);}
trap13() { signal(13,trap13); trap(13);}
trap14() { signal(14,trap14); trap(14);}
trap15() { signal(15,trap15); trap(15);}
trap16() { signal(16,trap16); trap(16);}

int	(*trapadr[])() =
{trap0 ,trap1 ,trap2 ,trap3 ,trap4 ,trap5 ,trap6 ,trap7 ,trap8,
 trap9 ,trap10,trap11,trap12,trap13,trap14,trap15,trap16};

trap(n)			/*  Trap action  */
register int	n;
BEGIN
    register int pipefd[2], child_id, savint = interactive, result = 0;

    IF trapcmd[n] THEN
        pipe(pipefd);
        IF (child_id = fork() ) == 0 THEN
            IF write(pipefd[1], trapcmd[n], strlen(trapcmd[n]) )
		!= strlen(trapcmd[n]) THEN
		perror("shell");
	    FI
            IF write(pipefd[1], "\n", 1) != 1 THEN
		perror("shell");
	    FI
            exit(0);
        ELIF child_id != -1 THEN
            putwait(child_id);
            clfile(pipefd[1]);
            interactive = FALSE;
            result = parsex(pipefd[0], SUBCMD);
            clfile(pipefd[0]);
            interactive = savint;
        FI
    FI
    return(result);
END

trapsig(n, s)		/* Set up trap  */
register int n; register char *s;
BEGIN
    IF s == NIL THEN
        defsig(n);
    ELSE
        IF trapcmd[n] THEN free(trapcmd[n]) FI
        trapcmd[n] = (char *)calloc(strlen(s)+1, sizeof(char) );
        strcpy(trapcmd[n], s);
	IF *trapcmd[n] == C_NUL THEN
		signal(n,SIG_IGN);
	ELSE
        	signal(n,trapadr[n]);
	FI
    FI
END

trapinit()		/*  Clear all traps  */
BEGIN
    register int i, (*slask)();
    FOR i=0; i<NSIG; ++i DO
        trapcmd[i] = NIL;
        signal(i, oldsig[i] = signal(i, slask) );
    OD
END

showtrap()
BEGIN
    register int i;
    FOR i=0; i<NSIG; i++ DO
        IF trapcmd[i] THEN
            prbrk(itoa(i));prbrk(": ");prline(trapcmd[i]);
        FI
    OD
END

defsig(n)		/*  Default signal action  */
register int	n;
BEGIN
    signal(n,oldsig[n]);
    IF trapcmd[n] THEN free(trapcmd[n]); trapcmd[n] = NIL; FI
END

INTPTR	waitlist = (INTPTR)NIL;
int waiting = FALSE;

itrpt()			/*  SIGINT handling  */
BEGIN
    signal(SIGINT, itrpt);		/*  Reenable trap  */
    IF waiting THEN waiting = FALSE; return FI
    waitfor(-1, TRUE);			/*  Wait for children  */
    IF interactive ANDF !(flags&T_FLAG) THEN
    	prline("");	
    	longjmp(mainenv, 0);		/*  To command level  */
    FI
    exit(SIGINT);
END

/*  Keep track of all children worth waiting for  */


putwait(id)		/*  Put child id on waiting list  */
register int id;
BEGIN
    register INTPTR node, p;
    
    node = (INTPTR)calloc(INTSIZ, sizeof(char));
    node->intnxt = (INTPTR)NIL;
    node->intval = id;
    
    IF !waitlist THEN
        waitlist = node;
    ELSE
        FOR p=waitlist; p->intnxt; p=p->intnxt DO
        OD
        p->intnxt = node;
    FI
END

remwait(id)			/*  Remove process id from waitlist  */
register int	id;
BEGIN
    register INTPTR this, *last;
    
    this = waitlist;
    last = &waitlist;
    
    WHILE this ANDF this->intval != id DO
        last = &(this->intnxt);
        this = this->intnxt;
    OD
    
    IF this THEN
        *last = this->intnxt;
        free(this);
    FI
END

int	waitfor(id, ignbgr)	/*  Wait for process #id to die. If id=-1  */
register int id, ignbgr;		/*  then wait for all children. If ignbgr  */
BEGIN				/*  is TRUE then ignore background child-  */
    int status;			/*  ren (they are not on waitlist).        */
    register int i;

    WHILE ignbgr ? (waitlist != (INTPTR)NIL) : TRUE DO
	waiting = TRUE;
	i = wait(&status);
	IF i > 0 THEN remwait(i) FI
	IF !waiting THEN itrpt(); ELSE waiting = FALSE; FI
	IF i == -1 THEN
            clrwait(); return(0);
        ELSE
		register int	sys_sig = status&0177;
		register int	st_high = (status>>8)&0177;
		IF sys_sig THEN
			IF sys_sig == 0177 THEN
			fprintf(stderr, ER_PTRACE);
			sys_sig = st_high;
			FI
			IF sys_msg[sys_sig] THEN
				IF i != id ORF interactive THEN
				fprintf(stderr, "%d %s ", i, sys_msg[sys_sig]);
				FI
				IF status & 0200 THEN fprintf(stderr, ER_CORED); FI
			FI
			fprintf(stderr,"\n");
		FI
            IF i == id THEN
                return(status);
            FI
        FI
    OD
END


clrwait()		/*  Clear waitlist  */
BEGIN
    register INTPTR p, q;
    
    p = waitlist;
    WHILE p DO
        q = p->intnxt;
        free(p);
        p = q;
    OD
    waitlist = (INTPTR)NIL;
END
