/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	====================================
*
*	== init.c ==	Initialize
*
*	====================================
*
*	Edit 002	1983-07-21
*/

#include	"defs.h"

extern		stackinit();
extern		cmdbufinit();
extern		varinit();
extern		trapinit();
extern	char	*itoa();
extern	char	**environ;
extern	char	**setenv();
int		dead_end = FALSE;

#ifdef	M_V7
#define	strrchr		rindex
#endif

init(argc,argv)
register int argc;
register char *argv[];
BEGIN
    stackinit();
    cmdbufinit();
    varinit();
    getenv();
    trapinit();
	BEGIN
		char *strrchr(), *p;
		setvar("0", (p = strrchr(argv[0],'/')) ? p + 1 : argv[0]);
	END

END

getenv()
BEGIN
    register char **e = environ, *p;
	environ = (char **)NULL;
    FOR ; *e; e++ DO
        FOR p = *e; *p != '='; ++p DO OD
        *p = C_NUL;
        setvar(*e, p+1);
        markvar(*e, EXPORT);
        *p = '=';
    OD
    environ = (char **)NULL;
    setenv();
END
