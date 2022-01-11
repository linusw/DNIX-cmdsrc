/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	==============================================
*
*	== x_simple.c ==	Execute simple command.
*				(This routine forks,
*				 sets up the environment,	
*				 and calls execute(). )
*	==============================================
*
*	Edit 008	1983-07-29
*/

#include "defs.h"

extern	char	*itoa();
extern	WORDPTR	argcrunch();
extern	int	execute();
extern		x_io();
extern		setvar();
extern	char	*unquote();
extern	char	*varsub();
extern	char	*cmdsub();
extern		putwait();
extern		remwait();
extern	char	*trapcmd[NSIG];
extern	int	dead_end;
extern	int	interactive;
extern	int	flags;


x_simple(simp, input, output)
register SIMPPTR simp; register int input, output;
BEGIN
    register int c;
    register WORDPTR arglist;
    
PRINT("X_SIMPLE\n");
    /*---------------------------------------------------------*/
    /*  If no command, then execute assignments in this shell  */
    /*---------------------------------------------------------*/

    IF !(simp->simpcmd) THEN
        x_asgn(simp->simpasgn, FALSE);
        IF !(simp->simpio) THEN
            return(0);
        FI
	arglist = (WORDPTR)NIL;
    ELSE
	arglist = argcrunch(simp->simpcmd);
	IF flags&X_FLAG THEN show(arglist) FI
    	IF arglist ANDF (c = intrncode(arglist->wordtxt)) THEN
		IF c == I_LOGIN ORF c == I_NEWGRP THEN 
			dead_end = TRUE;
			simp->simpnxt = (NODPTR)NIL;
		ELSE
        		return(x_intrn(c, simp, arglist, input, output));
		FI
	FI
    FI
    return(spawnex(simp, arglist, input, output));    
END

/*----------------------------------------------------------------*/
/*  If this is the last node of this branch of the tree, and the  */
/*  dead_end flag is set, then just exec() and die.               */
/*  Else fork, let the child exec() and return it's exit status.  */
/*----------------------------------------------------------------*/

spawnex(simp, arglist, input, output)
register SIMPPTR simp;
register WORDPTR arglist;
register int input, output;
BEGIN    
    register int  wait_id, child_id, status;

PRINT("SPAWNEX\n");
    IF (dead_end ANDF !simp->simpnxt) ORF
	   (child_id = fork() ) == 0	THEN
        interactive = FALSE;
	/* Set signals to defaults if not trapped */
	IF !trapcmd[SIGINT] THEN defsig(SIGINT) FI
	IF !trapcmd[SIGQUIT] THEN defsig(SIGQUIT) FI
	IF !trapcmd[SIGTERM] THEN defsig(SIGTERM) FI
	redirect(0,input);
        redirect(1,output);
        x_asgn(simp->simpasgn, TRUE);		/*  Execute assignments  */
        IF x_io(simp->simpio) THEN		/*  Execute io nodes  */
            exit(execute(arglist, TRUE ));	/*  Execute command  */
        ELSE
            exit(-1);
        FI
    ELSE
	IF child_id == -1 THEN
	    perror("shell");
            return(-1);
        ELSE
                putwait(child_id);	/*  Put child on waiting list  */
                status = waitfor(child_id, TRUE);  /*  Wait for it to die  */
                IF (LOBYTE(status) ) THEN
                    return( 200+LOBYTE(status) );  /*  Abnormal termination  */
                ELSE
                    return( HIBYTE(status) );  	/*  Normal termination    */
                FI
	FI
    FI
END

x_asgn(asp, x)			/* Execute list of assignments. */
register ASGNPTR asp;		/* If x, then export them all.  */
register int	 x;
BEGIN

PRINT("X_ASGN\n");
    FOR  ; asp; asp = asp->asgnnxt DO
        setvar(unquote(asp->asgnname), unquote(cmdsub(varsub(asp->asgnval))));
        IF x THEN markvar(asp->asgnname, EXPORT) FI
    OD
END        
