/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	=======================================
*
*	== cmdbuf.c ==	command buffer handling
*
*	=======================================
*
*	Edit 006	1983-07-11
*
*	Modified by P-A Svensson
*	Dataindustrier AB T{by
*	Apr 1984
*/


#include "defs.h"
#include <sys/types.h>
#include <sys/stat.h>

extern	int	errno;
extern	int	interactive;
extern  int	skipex;
extern	char	*secvar, *blnkvar;
extern char	*getstack();
extern char	*calloc();
extern		 free();
extern	int	 flags;
extern	char	*ldpchar;
extern  char    *getvar();
int	infile = 0;
FILE *stream[FDMAX+1] = NULL;		/* ptrs to input streems */
char	*cmdbuf[BUFNUM] = NULL;    /*  ptrs to command buffers  */
int		 readbuf = 0;      /*  index of buffer being read  */
int		 getbuf = 0;       /*  index of buffer being filled  */
int		 readpos = 0;      /*  where to read in readbuf  */
int		 getpos = 0;       /*  where to write in getbuf  */
struct stat	statb;
time_t	mailtime;


char	charin(fd)
register int fd;
BEGIN
	register char	c;

	IF stream[fd] == NULL THEN
		/* streem not open */
		stream[fd] = fdopen(fd,"r");
	FI
trapsys:
	errno = 0;
	IF (c = getc(stream[fd])) == EOF THEN
		IF errno == EINTR THEN
			goto trapsys;		/* Trap while in system */
		FI
		c = C_EOF;
	ELIF flags & V_FLAG THEN putc(c, stderr) FI
	return(c);
END

clfile(fd)
register int fd;
BEGIN
/*	Close streamed or non streamd file */
	IF stream[fd] != NULL THEN
		fclose(stream[fd]);
		stream[fd] = NULL;
	ELSE
		close(fd);
	FI
END


cmdbufinit()	                       /*  allocate one buffer  */
BEGIN
    register int i;
    cmdbuf[0] = (char *)calloc(BUFLEN,sizeof(char));
    FOR i=1 ; i < BUFNUM ; i++ DO
        cmdbuf[i]=NIL;
    OD
    clearcmdbuf();
END

clearcmdbuf()				/*  free buffers 1 - BUFNUM, clear buffer 0  */
BEGIN
    register int i;
    FOR i = 1; cmdbuf[i] != NIL ANDF i < BUFNUM ; i++ DO
        	free(cmdbuf[i]);
        	cmdbuf[i] = NIL;
    OD
    readbuf=getbuf=readpos=getpos=0;
END

char readchar(prompt)                    /*  read char from readbuf  */
register char	*prompt;
BEGIN
    register char c;
    register int quoting = FALSE;
    char *mailvar;
    
    IF readpos==BUFLEN THEN			/*  If end of buffer then  */
        readbuf++;				/*  read from next buffer  */
        readpos=0;
    FI
    IF readbuf==getbuf ANDF readpos==getpos THEN  /*  If no more to read  */
	IF interactive ANDF !(flags&T_FLAG) THEN
		/* 'You have mail.'-test */
		IF (mailvar = getvar("MAIL")) ANDF (*mailvar != 0) ANDF
			(stat(mailvar,&statb) >=0) ANDF
			(statb.st_mtime != mailtime) THEN
			IF statb.st_size THEN 
				prline("You have mail.");
			FI
			mailtime = statb.st_mtime;
		FI
        	fprintf(stderr,"%s",prompt);
	FI
        REPEAT
		/* Read chars from input. Handle single and multiple */
		/* character quoting by setting hi bit in quoted chars */
		IF ((c=charin(infile)) == C_QUOTE) ANDF !quoting THEN
			/* Single quotation - skip new line to allow */
			/* command to occupy more than one line      */
			IF (c = charin(infile)) == C_NL THEN goto skip_nl; ELSE
				c |= QUOTE;
			FI
		ELIF c == C_SQUOT THEN
			/* Multiple quotation - toggle quote flag, save */
			/* quoute character unquoted                    */
			IF quoting THEN
				quoting = FALSE;
			ELSE
				quoting = TRUE;
			FI
		ELIF quoting THEN
			/* in quote mode - set hi bit */
			c |= QUOTE;
		FI
		*(cmdbuf[ getbuf ] + getpos++) = c;
    		IF getpos == BUFLEN THEN
			IF getbuf < BUFNUM THEN
            		cmdbuf[++getbuf] = (char *)calloc(BUFLEN,sizeof(char));
            		getpos = 0;
        		FI
		FI
	skip_nl:
		IF quoting THEN
			IF (c & UNQUOTE) == C_NL ANDF interactive THEN
				fprintf(stderr,"%s",secvar);
			ELIF c == C_EOF THEN
				fatalerr(ER_EOF);
			FI
		FI
        UNTIL ( c==C_NL ORF c==C_EOF ) ANDF !quoting  DONE
    FI
    return( *(cmdbuf[readbuf] + readpos++));
END

skipblanks(prompt)			/*  skip blanks in cmdbuf  */
register char	*prompt;
BEGIN
    register char	*blanks = blnkvar;
    
    WHILE instr(" \t\n",readchar(prompt)) >= 0 DO OD
    setcmdpos(cmdpos()-1);
END

skipblen(prompt)			/*  Skip blanks except newline  */
register char	*prompt;
BEGIN
    register char c, *blanks = blnkvar;
    
    WHILE  instr(" \t\n", c = readchar(prompt)) >= 0 ANDF c != C_NL DO OD
    setcmdpos(cmdpos()-1);
END


char *cmdcop(start,stop,copy)	/*  return pointer to subcommand  */
register int start,stop,copy;		/*  or to copy of it  */

BEGIN
    register int buf,pos;
    register char *s,*p;
    
    IF start==stop THEN return("\0") FI
    buf = start / BUFLEN;
    pos = start % BUFLEN;
    IF copy THEN
        s = p = getstack(stop-start+1);
        
        WHILE (buf*BUFLEN+pos) < stop DO
            IF pos==BUFLEN THEN
                buf++ ; pos=0;
            FI
            *p++ = *(cmdbuf[buf] + pos++);
        OD
        *p = '\0';			/*  terminate with nullbyte  */
        return(s);
    ELSE return(cmdbuf[buf]+pos);	/*  return pointer into buffer  */
    FI
END

char	nextchar(prompt)		/*  Next char to be read  */
register char	*prompt;
BEGIN
	register char	c = readchar(prompt);
	setcmdpos(cmdpos()-1);
	return(c);
END

