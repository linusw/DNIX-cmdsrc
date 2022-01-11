/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	==============================================
*
*	== x_intrn.c ==	Execute internal shell command
*
*	==============================================
*
*	Edit 001	1983-07-25
*
*	Modified by P-A Svensson
*	Dataindustrier AB T{by
*	Apr 1984
*/

#include "defs.h"

extern		prbrk();
extern		prline();
extern	char	*homevar;
extern	int	setflag();
extern		resetflag();
extern	VARPTR	varlist;
extern	char	*itoa();
extern	char	**setenv();
extern		clrpospar();
extern		errbrk();
extern		errline();
extern		x_asgn();
extern	int	execute();
extern	int	x_simple();
extern		shift();
extern	int	contlevel;
extern	int	breaklevel;
extern	int	interactive;
extern	int	dead_end;
extern	int	xecstat;
extern	char	*ldpchar,  engchar[], swechar[];
extern	char	*rsrvword, engword[], sweword[];
extern	char	*specchar, engspch[], swespch[];
extern	char	maxlc, maxuc;

x_intrn(c, node, arglist, input, output)
register int c, input, output;
register SIMPPTR node; register WORDPTR arglist;
BEGIN
    register	WORDPTR	ap;
    register	ASGNPTR	asp;
    register	VARPTR	vp;
    register	int     i;
    register	char	*s;
    register	char	*ss;
    
PRINT("X_INTRN\n");
    SWITCH c IN
        CASE I_NOP:
            return(0);
        CASE I_SET:
            ap = arglist->wordnxt;
            IF !ap THEN			/*  no args - display all variables  */
                vp = varlist->varnxt;
                WHILE vp DO
                    prbrk(vp->varnam); prbrk("="); prline(vp->varval);
                    vp = vp->varnxt;
                OD
            ELIF *(s = (ap->wordtxt) ) == '-' THEN
                IF !*(++s) THEN			/*  'set -'  */
                    resetflag('v');
                    resetflag('x');
                ELSE
                    WHILE *s DO
	                    IF !setflag(*s++) THEN
	                        prbrk(ap->wordtxt); prline(ER_BOP);
				return(0);
	                    FI
			OD
                FI
                ap = ap->wordnxt;
            FI
            IF ap THEN
                clrpospar();
                FOR i=0; ap; ap = ap->wordnxt DO
                    setvar(itoa(++i), ap->wordtxt);
                OD
                setvar(PARVAR,itoa(i) );
            FI
            return(0);
	CASE I_READ:
		BEGIN
		char inbuf[256];
		register char ch;
		int result;
		FOR i=0; i < 256; i++ DO
			ch = charin(input);
			IF (ch == C_EOF) ORF (ch == C_NL) THEN
				result = (ch == C_EOF ? C_EOF : 0);
				inbuf[i] = '\000';
				break;
			FI
			inbuf[i] = ch;
		OD
		ap = arglist->wordnxt;
		s = inbuf;
		WHILE ap DO
			WHILE *s == ' ' DO s++ OD;
			ss = s;
			IF ap->wordnxt THEN
				WHILE (*ss != ' ') ANDF (*ss != '\000') DO 
					ss++;
				OD
				IF *ss == '\000' THEN
					*(ss+1) = '\000';
				ELSE
					*ss = '\000';
				FI
			FI
			IF is_varname(ap->wordtxt) THEN
				setvar(ap->wordtxt,s);
			ELSE
				errbrk(ap->wordtxt);errline(ER_ILLVAR);
				BREAK
			FI
			s = ++ss;
			ap = ap->wordnxt;
		OD
		return(result);
		END;
        CASE I_CD:
            IF arglist->wordnxt THEN
                s = arglist->wordnxt->wordtxt;
            ELSE
                s = homevar;
            FI
            IF chdir(s) THEN
                perror(s);
                return(-1);
            ELIF node->simpio THEN
                prline(ER_ILLIO);
                return(-1);
            ELSE return(0);
            FI
        CASE I_EXPORT:
        CASE I_RDONLY:
            ap = arglist->wordnxt;
		IF !ap THEN		/*  no args - display variables  */
			vp = varlist;
			WHILE vp DO
				IF (c == I_EXPORT ? vp->varexp : vp->varrdonly) ANDF *vp->varnam THEN
					prbrk(vp->varnam); prbrk("="); prline(vp->varval);
				FI
				vp = vp->varnxt;
			OD
		ELSE
			WHILE ap DO
				IF is_varname(ap->wordtxt) THEN
				markvar(ap->wordtxt, c == I_EXPORT? EXPORT : RDONLY);
				ELSE
					errbrk(ap->wordtxt); errline(ER_ILLVAR);
					BREAK
				FI
				ap = ap->wordnxt;
			OD
			IF c == I_EXPORT THEN setenv(); FI
		FI
		return(0);
        CASE I_PARSEX:
            IF node->simpio THEN
                errline("Illigal io.");
                return(-1);
            ELSE
                return(execute(arglist->wordnxt, FALSE));
            FI
        CASE I_CONT:
        CASE I_BREAK:
            IF (ap = arglist->wordnxt) THEN	   /*  If argument  */
                IF (i = atoi(ap->wordtxt) ) < 0 THEN
                    errbrk(s);
                    errline(ER_BNUM);
                    return(-1);
                FI
            ELSE
                i = 1;
            FI
            IF c == I_CONT THEN
                contlevel = i;
            ELSE
                breaklevel = i;
            FI
            return(0);                
        CASE I_EVAL:
            BEGIN
                register int pipefd[2], savint = interactive;
                pipe(pipefd);
                IF (i = fork()) == 0 THEN
                    FOR ap = arglist->wordnxt; ap; ap=ap->wordnxt DO
                        write(pipefd[1],ap->wordtxt, strlen(ap->wordtxt) );
                        write(pipefd[1]," ",1);
                    OD
                    write(pipefd[1],"\n",1);
                    exit(0);
                ELIF i != -1 THEN
                    putwait(i);
                    clfile(pipefd[1]);
                    interactive = FALSE;
                    i = parsex(pipefd[0],SUBCMD);
                    clfile(pipefd[0]);
                    interactive = savint;
                    return(i);
                ELSE
                    perror("shell");
                    return(-1);
                FI
            END
        CASE I_EXEC:
            IF (node->simpcmd = node->simpcmd->wordnxt) THEN
		dead_end = TRUE;
                node->simpnxt = (NODPTR)NIL;
                return(x_simple(node, input, output));
            ELSE
                x_asgn(node->simpasgn, FALSE);
                return(!x_io(node->simpio));
            FI   
        CASE I_EXIT:
            IF interactive THEN return(0);		/*  If interactive do nothing  */
            ELIF (ap = arglist->wordnxt) THEN		/*  Elif numeric argument  */
                IF (i = atoi(ap->wordtxt) ) < 0 THEN
                    errbrk(s);
                    errline(ER_BNUM);
                    return(-1);
                ELSE
                    exit(i);				/*  then exit with that as exit status  */
                FI
            ELSE					/*  Else exit with exit status  */
                exit(xecstat);				/*  of last command executed  */
            FI
        CASE I_TIMES:
            BEGIN
                long int t[4];
                times(t);
                prtime(t[2]); prtime(t[3]); prline("");
            END
            return(0);
        CASE I_SHIFT:
            shift();
            return(0);
        CASE I_UMASK:
            IF arglist->wordnxt THEN
                IF (i=otoi(arglist->wordnxt->wordtxt) ) >= 0 THEN umask(i) FI
            ELSE
                prline(itoo(umask() ) );
            FI
            return(0);
        CASE I_WAIT:
            return( waitfor( arglist->wordnxt ? atoi(arglist->wordnxt->wordtxt) : -1, FALSE) );
        CASE I_TRAP:
            BEGIN
                register int n; register char* command = NIL;
                IF !arglist->wordnxt THEN showtrap();
                ELSE
                    IF !is_digit(*(arglist = arglist->wordnxt)->wordtxt) THEN
                        command = arglist->wordtxt;
                        arglist = arglist->wordnxt;
                    FI
                    FOR ; arglist; arglist = arglist->wordnxt DO
                        IF (n = atoi(arglist->wordtxt)) >= 0 ANDF n < NSIG THEN
                            trapsig(n, command);
                        ELSE
                            errbrk(arglist->wordtxt);errline(ER_BNUM);
                            return(-1);
                        FI
                    OD
                FI
                return(0);
            END
        CASE I_SWE:
            ldpchar  = swechar;
            rsrvword = sweword;
            specchar = swespch;
            maxlc    = '}';
            maxuc    = ']';
	    setvar(LANGVAR,"swedish");
            return(0);
        CASE I_ENG:
            ldpchar  = engchar;
            rsrvword = engword;
            specchar = engspch;
            maxlc    = 'z';
            maxuc    = 'Z';
	    setvar(LANGVAR,"english");
            return(0);
    ENDSW
END

int is_varname(s)
register char	*s;
BEGIN
    IF !is_letter(*s) THEN return(FALSE) FI
    WHILE *(++s) DO
        IF !is_letter(*s) ANDF !is_digit(*s) THEN return(FALSE) FI
    OD
    return(TRUE);
END
