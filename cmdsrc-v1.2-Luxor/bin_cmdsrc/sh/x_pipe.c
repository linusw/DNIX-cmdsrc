/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	====================================
*
*	== x_pipe ==	Execute pipe node
*
*	====================================
*
*	Edit 002	1983-07-08
*/

#include	"defs.h"

extern	int	x_cmdlist();
extern		redirect();
extern		putwait();
extern	int	interactive;
extern	int	dead_end;


int	x_pipe(node, input, output)
register PIPEPTR node; register int input, output;
BEGIN
    register int child_id, hold, value, pipefd[2];
    
PRINT("X_PIPE\n");
    IF pipe(pipefd) THEN						/*  Create pipe  */
        perror("shell");
        return(-1);
    FI
    
    /*  Fork into one process that writes the pipe,  */
    /*  and one that reads the pipe.                 */
    /*  Return the exit status of the latter.        */
    IF (child_id = fork() ) == 0 THEN					/*  The writing child  */
        interactive = FALSE;
        /* dead_end = TRUE; TEMPORARY REMOVE */
        clfile(pipefd[0]);
        exit(x_cmdlist(node->pipein, input, pipefd[1]) );		/*  Execute and die  */
        
    ELIF child_id == -1 THEN							/*  The fork aborted  */
        IF interactive THEN perror("shell") FI
        clfile(pipefd[0]);							/*  Close the pipe  */
        clfile(pipefd[1]);
        return(-1);
    ELSE									/*  The reading parent  */
        putwait(child_id);
        clfile(pipefd[1]);
        value =(x_cmdlist(node->pipeout, pipefd[0], output));		/*  Execute  */
        clfile(pipefd[0]);							/*  Close the pipe  */
        return(value);
    FI
END
