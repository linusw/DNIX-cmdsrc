/*
 * Copyright (C) Dataindustrier AB, 1983
 */

/*
 *	Init process, pid=1  	(parent of all processes)
 *	Kill commands:
 *	    kill -2 1 (SIGINT) to force re-examination of /etc/ttys
 *	    kill -1 1 (SIGHUP) to force reset (return to single user mode)
 *	    kill -3 1 (SIGQUIT) to suppress `getty' on logouts
 *	    kill    1 (SIGTERM) to halt the system
 */

#include <signal.h>
#include <sys/types.h>
#include <utmp.h>
#include <sgtty.h>
#include <errno.h>
#include <dnix/fcodes.h>

#define	LINSIZ	sizeof(wtmp.ut_line)
#define CMDSIZ 120
#define MAXARG 30
#define WAITGO 4
#define	FOUND	1
#define	CHANGE	2
# ifdef	MAXUSERS
#define	TABSIZ	MAXUSERS
# else
#define	TABSIZ	100
# endif
#define	ALL	p = &itab[0]; p < &itab[TABSIZ]; p++
#define	EVER	;;
#define SCPYN(a, b)	strncpy(a, b, sizeof(a))
#define SCMPN(a, b)	strncmp(a, b, sizeof(a))

char	shell[]	= "/bin/sh";
char	getty[]	 = "/bin/stty";
char	minus[]	= "-";
char	runc[]	= "/etc/rc";
char	ifile[]	= "/etc/ttys";
char	efile[] = "/etc/ttys.ext";
char	utmp[]	= "/etc/utmp";
char	mtab[] = "/etc/mtab";
char	wtmpf[]	= "/usr/adm/wtmp";
char	ctty[]	= "/dev/console";
char	dev[]	= "/dev/";

struct utmp wtmp;
struct
{
	char	line[LINSIZ];
	char	ttycmd[CMDSIZ];
	char	comn;
	char	flag;
} line;
struct	tab
{
	char	line[LINSIZ];
	char	ttycmd[CMDSIZ];
	char	comn;
	char	xflag;
	int	pid;
} itab[TABSIZ];

/*eject*/
int	fi;
int     fe;
int	mergflag;
int	quitflag;
int	gosingle;
extern	int	errno;
char	tty[20];

int	setsig();
int	haltsys();
char	*strcpy(), *strcat();
long	lseek();

/*eject*/
main()
{
	sync();
	signal(SIGTERM,haltsys);
	for(EVER) {
		singusr();	/* Enter single user mode */
		runrc();	/* run /etc/rc - shell script */
		rdttys();	/* Inspect /etc/ttys */
		multusr();	/* Enter multi user mode */
		stopsys();	/* Stop system */
	}
}

/*eject*/
singusr()
{
/*
 *	Enter single user mode
 */
	register pid;
	struct sgttyb dumioc;

	pid = fork();
	if(pid == 0) {
		signal(SIGHUP, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		signal(SIGALRM, SIG_DFL);
		open(ctty, 2);
		ioctl(0,SF_CTER,&dumioc);	/* Set controlling terminal */
		dup(0);
		dup(0);
		if ( (gosingle) || (autosw() == 0) )
			execl(shell, minus, (char *)0);
		exit(0);
	}
	while(wait((int *)0) != pid)
	;
}
/*eject*/
runrc()
{
/*
 *	Run shell script /etc/rc
 */
	register pid, f;

	pid = fork();
	if(pid == 0) {
		open("/", 0);
		dup(0);
		dup(0);
		execl(shell, shell, runc, (char *)0);
		exit(0);
	}
	while(wait((int *)0) != pid)
	;
	f = open(wtmpf, 1);
	if (f >= 0) {
		lseek(f, 0L, 2);
		SCPYN(wtmp.ut_line, "~");
		SCPYN(wtmp.ut_name, "");
		time(&wtmp.ut_time);
		write(f, (char *)&wtmp, sizeof(wtmp));
		close(f);
	}
}
/*eject*/
rdttys()
{
/*
 *	Inspect tty file /etc/ttys
 */
	register struct tab *p;

	fi = open(ifile, 0);
	if(fi < 0)
		return;
	fe = open(efile, 0);
	for(ALL)
		p->xflag = 0;
	while(rline()) {
		for(ALL) {
			if (SCMPN(p->line, line.line))
				continue;
			p->xflag |= FOUND;
			if(line.comn != p->comn) {
				p->xflag |= CHANGE;
				p->comn = line.comn;
			}
			goto contin1;
		}
		for(ALL) {
			if(p->line[0] != 0)
				continue;
			SCPYN(p->line, line.line);
			SCPYN(p->ttycmd,line.ttycmd);
			p->xflag |= FOUND|CHANGE;
			p->comn = line.comn;
			goto contin1;
		}
	contin1:
		;
	}
	close(fi);
	close(fe);
	for(ALL) {
		if((p->xflag&FOUND) == 0) {
			if (p->line[0])
				term(p);
			freetty(p);
			p->line[0] = 0;
		}
		if((p->xflag&CHANGE) != 0) {
			term(p);
			dfork(p,0);
		}
	}
}
/*eject*/
multusr()
{
/*
 *	Enter multi user mode
 */
	register struct tab *p;
	register pid;

	quitflag = 0;
	gosingle = 0;
loop:
	mergflag = 0;
	signal(SIGINT, setsig);
	signal(SIGQUIT, setsig);
	signal(SIGHUP,setsig);
	for(EVER) {
		pid = wait((int *)0);
		if(mergflag) {
			rdttys();
			goto loop;
		}
		if(gosingle){
			return;
		}
		if (pid == -1)
			if (errno == EINTR) {
				errno = 0;
				goto loop;
			} else
				return;
		for(ALL)
			if(p->pid == pid || p->pid == -1) {
				logout(p);
				if (! quitflag)
					dfork(p,WAITGO);
			}
	}
}
/*eject*/
stopsys()
{
/*
 *	Kill all processes not booted up at start of system
 */
	register i;
	register struct tab *p;

	close(creat(utmp, 0644));
	close(creat(mtab,0644));
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	for(ALL) {
		term(p);
		freetty(p);
		p->line[0] = 0;
	}
	kill(-2,SIGKILL);	/* Kill activ processes */
	sleep(3);
	kill(-3,SIGTERM);	/* Kill handlers */
	i  = 0;
	while(i < 20 && kill(-3,0) == 0){
		i++;
		if(i == 7){
			kill(-3,SIGTERM);
		}
		if(i == 14){
			kill(-3,SIGKILL);
		}
		sleep(1);
	}
	/* Kill all non system processes */
	kill(-1,SIGTERM);
	sleep(2);
	for(i = 0; i < 10; i++){
		close(i);
	}
}
/*eject*/
setsig(i)
{
/*
 *	Set flags: SIGINT - check /etc/ttys
 *		   SIGQUIT- do not allow logins to be started
 *		   SIGHUP - enter single user mode
 */
	if(i == SIGINT){
		mergflag = 1;
	} else if(i == SIGQUIT){
		quitflag = 1;
	} else if(i == SIGHUP){
		gosingle = 1;
	}
}
/*eject*/
term(p)
register struct tab *p;
{
/*
 *	Terminate process using this terminal - mark user logged out
 */
	if(p->pid != 0) {
		logout(p);
		kill(p->pid, SIGKILL);
	}
	p->pid = 0;
}

freetty(p)
register struct tab *p;
{
/*
 *	Disconnect terminal - change it's mode and owner
 */
	register int fn;

	if (p->line[0]) {
		strcpy(tty, dev);		/* assemble tty-name */
		strncat(tty, p->line, LINSIZ);
		chown(tty,0,0);		/* Change owner */
		chmod(tty,0666);	/* Change mode */
	}
}
/*eject*/
rline()
{
/*
 *	Read next line in /etc/ttys and /etc/tty.ext
 */
	register c, i;

loop:
	c = get(fi);
	if(c < 0)
		return(0);
	if(c == 0)
		goto loop;
	line.flag = c;
	c = get(fi);
	if( c <= 0 ) goto loop;
	line.comn = c;
	c = get(fi);
	SCPYN(line.line, "");
	for (i=0; i<LINSIZ; i++) {
		if( (c <= 0) || (c == ' ') )
			break;
		line.line[i] = c;
		c = get(fi);
	}
	while(c > 0) c = get(fi);
	if(line.line[0] == 0)
		goto loop;
	if(line.flag == '0')
		goto loop;
	strcpy(tty, dev);
	strncat(tty, line.line, LINSIZ);
	if(access(tty, 06) < 0)
		goto loop;
	if( fe < 0 ) goto deflt;
	lseek(fe,(long )0,0);
retry:
	c = get(fe);
	if( c < 0 ) goto deflt;
	if( c == 0 ) goto retry;
	for (i=0; i<LINSIZ; i++) {
		if(c != line.line[i]) break;
		c = get(fe);
	}
	if( (line.line[i]  != ' ') && ( line.line[i] != '\0') ) goto skipln;
	if( c != ' ' ) goto skipln;
        c = get(fe);
	SCPYN(line.ttycmd,"");
	for(i=0; i<CMDSIZ; i++) {
		if(c <= 0) break;
		line.ttycmd[i] = c;
		c = get(fe);
	}
	if( line.ttycmd[0] == 0 ) { 
skipln:		if (c <= 0) goto retry;
		while((c=get(fe)) > 0);
		goto retry;
	}
	return(1);

deflt:
	strcpy(line.ttycmd,"/bin/login -");
	return(1);
}

get(fd)
int fd;
{
	char b;

	if(read(fd, &b, 1) != 1)
		return(-1);
	if(b == '\n')
		return(0);
	return(b);
}
/*eject*/
dfork(p,dly)
struct tab *p;
int dly;
{
/*
 *	Start a process for given terminal
 */
	register pid;
	struct sgttyb dumioc;
	char *argcmd[MAXARG];
	char cmd[CMDSIZ+LINSIZ+3];
	register i,n;

	pid = fork();
	if(pid == 0) {
		signal(SIGHUP, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		strcpy(cmd,"- ");
		tty[0] = p->comn;
		tty[1] = ' ';
		tty[2] = 0;
		strcat(cmd,dev);
		strcat(cmd, p->line);
		strcat(cmd," ");
		strcat(cmd,tty);
		strcat(cmd,p->ttycmd);
		i = n = 0;
		while( i < MAXARG ) {
			argcmd[i++] = &cmd[n++];
			while( (cmd[n] != ' ') && (cmd[n] != 0) )
				n++;
			if( cmd[n] == ' ' ) 
				cmd[n++] = 0;
			while( cmd[n] == ' ' ) 
				n++;
			if( cmd[n] == 0 )
				break;
		}
		argcmd[i] = 0;
		if( dly ) sleep(dly);
		execv(getty,argcmd);
		exit(0);
	}
	p->pid = pid;
}
/*eject*/
logout(p)
register struct tab *p;
{
/*
 *	User is logged out, remove entry in utmp and add
 *	accounting info in wtmp
 */
	register f;

	f = open(utmp, 2);
	if(f >= 0) {
		while(read(f, (char *)&wtmp, sizeof(wtmp)) == sizeof(wtmp)) {
			if (SCMPN(wtmp.ut_line, p->line))
				continue;
			lseek(f, -(long)sizeof(wtmp), 1);
			SCPYN(wtmp.ut_name, "");
			time(&wtmp.ut_time);
			write(f, (char *)&wtmp, sizeof(wtmp));
		}
		close(f);
	}
	f = open(wtmpf, 1);
	if (f >= 0) {
		SCPYN(wtmp.ut_line, p->line);
		SCPYN(wtmp.ut_name, "");
		time(&wtmp.ut_time);
		lseek(f, (long)0, 2);
		write(f, (char *)&wtmp, sizeof(wtmp));
		close(f);
	}
}
/*eject*/
haltsys()
{
/*
 *	Halt system
 */
	register int i;

	sync();
	stopsys();
	for(i = 4; i > 1; i--){
		kill(i,SIGTERM);
	}
	sleep(5);
	i = 0;
	while(i < 20 && kill(2,0) == 0){
		i++;
		sleep(1);
	}
	exit(0);
}

autosw()
{
/*
 *	Return TRUE if autoswitch = Yes else return FALSE
 */
	char buf[10];
	register int i,j;
	if( (j = open("dev/autosw",0)) < 0 ) {
		return(0) ;
	}
	i = read(j,buf,10);
	close(j);
	return( i>0 && ( (buf[0] == 'y') || (buf[0] == 'Y') ) );
}
