/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	=====================================
*
*	== filehdl.c ==		file handling
*
*	=====================================
*
*	Edit 003	1983-07-07
*/

#include	"defs.h"


int fileopen(name,mode,fd)		/*  Open fname as file fd in  */
register char *name,*mode;		/*  read/write mode ("r","w" or "a") */
register int fd;
BEGIN
    register int f, g;

    clfile(fd);			/*  in case fd was assigned to another file  */
    IF *mode == 'w' THEN
        f = creat(name,PMODE);
    ELIF *mode == 'a' THEN
        IF ( f = open(name,1)) == -1 THEN
            f = creat(name,PMODE);
        FI
        lseek(f,0L,2);
    ELSE
        f = open(name,0);
    FI
    IF f == fd ORF f == -1 THEN
        return(f);
    ELSE
        g = synon(fd,f);
        clfile(f);
        return(g);
    FI
END


redirect(fd1,fd2)		/* make fd1 synonymous to fd2  */
register int fd1, fd2;
BEGIN
    IF fd1 != fd2 THEN clfile(fd1) FI
    return(synon(fd1,fd2));
END



synon(fd1,fd2)
register int fd1,fd2;
BEGIN
    int f=fd1, g;
    IF fd1 == fd2 ORF (f = dup(fd2)) == fd1 ORF f < 0 THEN
        return(f);
    ELSE
        g = synon(fd1,fd2);
        clfile(f);
        return(g);
    FI
END
