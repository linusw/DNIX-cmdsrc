/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	================================================
*
*	== x_io.c ==		execute list of io-nodes
*
*	================================================
*
*	Edit 001	1983-06-22
*
*	Modified by P-A Svensson
*	Dataindustrier AB T{by
*	Apr 1984
*/

#include	"defs.h"

extern	char	*cmdcop();
extern	int	strlen();
extern		redirect();
extern		putwait();
extern	char	*varsub();
extern	char	*cmdsub();


x_io(iop)
register IOPTR	iop;
BEGIN
    
    register int buflen,fildes[2],piper_id;
    register char *p;

PRINT("X_IO\n");
    WHILE iop DO
        IF iop->iotxt && (p = (cmdsub(varsub(iop->iotxt)))) THEN
		iop->iotxt = p;
	FI
        SWITCH iop->iotype IN
            CASE INPTYPE:
                IF fileopen(iop->iotxt,"r",iop->iofd1) == -1 THEN
                    errbrk(iop->iotxt); errline(ER_CNOP);
                    return(FALSE);
                FI
                BREAK;
            CASE OUTTYPE:
                IF fileopen(iop->iotxt,"w",iop->iofd1) == -1 THEN
                    errbrk(iop->iotxt); errline(ER_CNOP);
                    return(FALSE);
                FI
                BREAK;
            CASE APPTYPE:
                IF fileopen(iop->iotxt,"a",iop->iofd1) == -1 THEN
                    fprintf(stderr,"%s: %s\n",iop->iotxt,ER_CNOP);
                    return(FALSE);
                FI
                BREAK;
                
            CASE HDOCTYPE:
                buflen = strlen(iop->iohdoc);
                IF pipe(fildes) THEN
                    perror("shell");
                    return(FALSE);
                FI
                IF (piper_id = fork() ) == 0 THEN		/* Child */
                    IF write(fildes[1],iop->iohdoc,buflen) != buflen THEN
                        perror("shell");
                    FI
                    exit(0);
                ELSE
                    putwait(piper_id);				/* Mother */
                    clfile(fildes[1]);
                    redirect(iop->iofd1,fildes[0]);
                    clfile(fildes[0]);
                FI
                BREAK;
            CASE INDUPTYPE:
            CASE OUTDUPTYPE:
                redirect(iop->iofd1, iop->iofd2);           
                BREAK;
            CASE CLINPTYPE:
            CASE CLOUTTYPE:
                clfile(iop->iofd1);
        ENDSW
        
        iop = iop->ionxt;
    OD
    return(TRUE);
PRINT("END X_IO\n");
END
