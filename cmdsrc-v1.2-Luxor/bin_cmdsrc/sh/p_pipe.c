/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	================================
*
*	== p_pipe.c ==	Parse a pipeline
*
*	================================
*
*	Edit 001	1983-10-02
*/

#include	"defs.h"
#include	"cmdbuf.h"

extern	NODPTR	p_cmd();
extern	int	p_rsrv();

NODPTR	p_pipe(must)
register   int	must;
BEGIN
    register    int		start, pos;
    PIPEPTR	p;
    register    NODPTR	inptr, outptr;

PRINT("P_PIPE\n");
    start = cmdpos();
    IF (inptr = p_cmd(must) ) THEN
        pos = cmdpos();
        IF p_rsrv(FALSE) != R_PIPE THEN
            setcmdpos(pos);
            return(inptr);
        ELIF outptr = p_pipe(TRUE) THEN
            p = (PIPEPTR)getstack(PIPESIZ);
            p->pipetype = PIPETYPE;
            p->pipeback = FALSE;
            p->pipenxt  = (NODPTR)NIL;
            p->pipein   = inptr;
            p->pipeout  = outptr;
            return( (NODPTR)p);
        FI
    FI
    setcmdpos(start);
    return( (NODPTR)NIL );
END
