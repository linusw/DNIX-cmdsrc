/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	============================================
*
*	== x_cmdlist.c ==	Execute command list
*
*	============================================
*
*	Edit 010	1983-07-16
*/

#include "defs.h"

extern	int	intrncode();
extern	int	x_intrn();
extern		setvar();
extern	int	x_simple();
extern	int	x_andor();
extern	int	x_pipe();
extern	int	x_if();
extern	int	x_for();
extern	int	x_while();
extern	int	contlevel;
extern	int	breaklevel;
extern	int	dead_end;
extern	int	interactive;
extern	int	flags;
extern	int	(*oldsig[])();
int		xecstat = 0;

x_cmdlist(node, input, output)
register NODPTR node; register int input, output;
BEGIN
    register int child_id, c;
    
PRINT("X_CMDLIST\n");
    xecstat = 0;
    WHILE node DO

        IF contlevel ORF breaklevel			/*  IF we are unnesting to     */
        THEN return(0);					/*  a 'continue' or a 'break'  */
        FI						/*  then don't execute         */	

/*------------------------------------------------------*/
/*  To execute a background command - spawn a child to  */
/*  do the job, then continue without waiting for it.   */
/*------------------------------------------------------*/

        IF node->nodeback THEN
            IF (child_id = fork() ) == -1 THEN		/*---------------*/
                perror("shell");			/*  Miscarriage  */
                return(-1);				/*---------------*/
            ELIF child_id THEN				
		IF interactive THEN fprintf(stderr,"%d\n",child_id) FI
                setvar(BAKVAR, itoa(child_id));		/*  Parent  */
                node = node->nodenxt;			/*----------*/
                continue;
            ELSE
		oldsig[SIGINT]	= SIG_IGN;
                oldsig[SIGQUIT] = SIG_IGN;
		defsig(SIGQUIT);defsig(SIGINT);
		dead_end = TRUE;
                node->nodenxt = (NODPTR)NIL;
		IF fileopen("/dev/null","r",0) == -1 THEN	/* Stdin = /dev/null */
			errbrk("/dev/null");errline(ER_CNOP);
			return(xecstat = -1);
		FI
            FI
        FI

        SWITCH node->nodetype IN
            CASE NULLTYPE:
                xecstat = 0;
                BREAK
            CASE SUBTYPE:
                IF ((SUBPTR)node)->subsub THEN
                    IF (child_id = fork()) == 0 THEN
                        exit(x_cmdlist(((SUBPTR)node)->subdo,input,output));
                    ELIF child_id == -1 THEN
                        perror("shell"); xecstat = -1;
                    ELSE 
                        putwait(child_id);
                        xecstat = waitfor(child_id, TRUE);
                    FI
                ELSE     
                    xecstat = x_cmdlist(((SUBPTR)node)->subdo,input,output);
                FI
                BREAK
            CASE SIMPTYPE:
                xecstat = x_simple(node, input, output);
                BREAK
            CASE PIPETYPE:
                xecstat = x_pipe(node, input, output);
                BREAK
                
            CASE ANDTYPE:
            CASE ORTYPE:
                xecstat = x_andor(node, input, output);
                BREAK
                
            CASE IFTYPE:
                xecstat = x_if(node, input, output);
                BREAK
                
            CASE FORTYPE:
                xecstat = x_for(node, input, output);
                BREAK
                
            CASE WHILTYPE:
                xecstat = x_while(node, input, output);
                BREAK
                
            CASE UNTILTYPE:
                xecstat = x_until(node, input, output);
                BREAK
                
            CASE CASETYPE:
                xecstat = x_case(node, input, output);
                BREAK
                
        ENDSW
        setvar(LASTVAR,itoa(xecstat) );
        IF xecstat ANDF !interactive ANDF (flags&E_FLAG) THEN exit(-1) FI
        node = node->nodenxt;
    OD
    IF dead_end THEN
        exit(xecstat);
    ELSE
        return(xecstat);
    FI    
END
