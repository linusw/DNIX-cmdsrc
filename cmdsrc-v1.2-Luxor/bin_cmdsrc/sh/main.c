/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	============================
*
*	== main.c ==
*
*	============================
*
*	Edit 010	1983-07-15
*/
#include	"defs.h"
#include	"cmdbuf.h"
#include	"errmess.h"

extern		init();
extern		parsex();
extern		itrpt();
extern	int	flags;
extern	int	infile;
extern  char *trapcmd[];

int		interactive = FALSE;
jmp_buf		mainenv;

main(argc, argv)
register int argc; char *argv[];
BEGIN
	int ttypar[5];
	int status;
	int profile;

PRINT("MAIN\n");
    init(argc, argv);		/*  Initialize storage etc  */
    signal(SIGQUIT,SIG_IGN);	/* Ignore quit signal */
    signal(SIGINT, itrpt);	/*  Catch interrupt signal  */
    infile = args(argc,argv);
    IF  argc ANDF *argv[0] == '-'
        ANDF
        (profile = open(".profile", 0) ) != -1 
    THEN
        parsex(profile, TOP);
        clfile(profile);
    FI
    setjmp(mainenv);			/*  Return here when disaster  */
    IF ( (gtty(infile,ttypar) == 0) ANDF (gtty(2,ttypar) == 0) ) ORF
                                    (flags & I_FLAG)  THEN
        interactive = TRUE;
	signal(SIGTERM,SIG_IGN);
    FI
    status = parsex(infile, TOP);
    IF trapcmd[0] THEN exit(trap(0)); FI
    exit(status);
END
