/*
 *	login command
 *
 *	Copyright (C) DIAB T{by 1983
 *	Written by: Per-Arne Svensson 
 */

/*
 *	login [name]
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <signal.h>
#include <sgtty.h>
#include <utmp.h>
#include <pwd.h>
#include "../cmd.h"
#include "../cmd_err.h"

/*	External functions						*/
/*	==================						*/
struct passwd *getpwnam();
int	setpwent();		/*	Reset and open password file	*/
int	endpwent();		/*	Close password file		*/
extern	char	**environ;	/*	Environment of this process	*/
extern	char	*dname();	/*	Get directory-name of file	*/
char	*ttyname();		/*	Get tty-name			*/
char	*crypt();		/*	Encryption routine		*/
char	*getpass();		/*	Get passwd from term or file	*/
char	*strcpy(), *strncpy();	/*	Copy a string			*/
char	*strcat(), *strncat();	/*	Concatenate strings		*/
int	strcmp();		/*	Compare strings			*/
int	strlen();		/*	Length of string		*/
int	alarm();		/*	Set time limit for process	*/
int	nice();			/*	Set process priority		*/
int	ttyslot();		/*	Get file pos of tty record	*/

/*	Global variables						*/
/*	================						*/

struct	passwd	nouser	= {"", "nope"};	/*	No_user password entry	*/
struct	passwd	*pwd;			/*	Password pointer	*/
struct	sgttyb	ttyblk;			/*	Terminal info_block	*/
struct	utmp	log_info;		/*	Record for login info	*/

/*		Environment definition					*/
char	path[]		= "PATH=:/bin:/usr/bin";
char	home[MAXN+5]	= "HOME=";
char	*envmnt[]	= {home, path, 0};

char	mail_dir[MAXN]	= MAIL_DIR;	/*	Mail directory		*/
char	sh_name[DIRSIZ+2] = "-";	/*	first argument to shell	*/

/*	Functions							*/
/*	=========							*/

int	motd();				/*	Read message of the day	*/
int	sig_alrm();			/*	Alarm signal trap	*/

/*	Main program							*/
/*	============							*/
main(argc, argv)
char *argv[];
int  argc;

{
char	*tty_name;		/*	Terminal name			*/
int	tty_fpos;		/*	Position in the tty-file	*/
int	filedsc;		/*	File descriptor			*/
char	*name_ptr;		/*	Login/Password character pointer*/
				/*	Login/Password name		*/
char	name[sizeof(log_info.ut_name)+1];
int	chr;			/*	Login character variable	*/
struct	stat	statblk;	/*	File status block		*/

	/*	Initiate prompt string					*/
	PRMPT = *argv;

	if (geteuid() != 0) {	/*	No superuser-permission		*/
		fprintf(stderr, SUONLY);
		exit(1);
	}

	--argc;
	++argv;

	/*	Ignore signals						*/
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGALRM, sig_alrm);


	/*	Set normal priority					*/
	nice(-40);
	nice(20);
	nice(0);

	/*	Get terminal status_block				*/
	ioctl(fileno(stdin), TIOCGETP, &ttyblk);

	/*	Set erase and kill character				*/
	ttyblk.sg_erase = ERASE_CH;
	ttyblk.sg_kill	= KILL_CH;

	/*	Update terminal status					*/
	ioctl(fileno(stdin), TIOCSETP, &ttyblk);

	/*	Close all files except stdin, stdout & stderr		*/
	for (filedsc = 0; filedsc < _NFILE; ++filedsc)
		if (filedsc != fileno(stdin) && 
			filedsc != fileno(stdout) && 
			filedsc != fileno(stderr))
			close(filedsc);

	/*	Position output stream to left margin			*/
	printf("\n");

	/*	Here starts the login-loop				*/
Login:
	/*	Initiate login-name					*/
	if (argc != 0) {
		SCPYN(name,  *argv);
		argc = 0;
	} else {
		SCPYN(name,  "");
		while (*name == '\0') {
			name_ptr = name;
			printf(LOGIN);
			while ((chr = getchar()) != '\n') {

				/*	Set time limit to 30 seconds	*/
				alarm(30);

				switch (chr) {
				case ' ':
					chr = '_';
					break;
				case EOF:
					exit(0);
				}
				if (strlen(name) < sizeof(name)-1)
					*name_ptr++ = chr;
			}
			/*	Terminate the string			*/
			*name_ptr = '\0';
		}
	}

	/*	Reset time limit of process				*/
	alarm(0);

	/*	Get tty_name						*/
	if ((tty_name = ttyname(fileno(stdin))) == NULL)
		tty_name = "/dev/tty??";

	/*	Copy name to the login record				*/
	SCPYN(log_info.ut_name, name);

	/*	Copy tty_name to the login record			*/
	SCPYN(log_info.ut_line, dname(tty_name));

	/*	Get passwd						*/
	if ((pwd = getpwnam(name)) == NULL)
		pwd = &nouser;

	/*	Set time limit to 30 seconds				*/
	alarm(30);

	/*	If there is any password, encrypt it and check		*/
	if (*pwd->pw_passwd != '\0') {
		name_ptr = crypt(getpass(PASSWD), pwd->pw_passwd);

		/*	If not correct start all over again		*/
		if (strcmp(name_ptr, pwd->pw_passwd) != 0) {
			/*	Login incorrect				*/
			printf(INCLOG);
			/*	Reset password file			*/
			setpwent();
			goto Login;
		}
	}

	/*	Close password file					*/
	endpwent();

	/*	Reset time limit of process				*/
	alarm(0);

	/*	Change directory, change GID and UID and init $HOME	*/
	if (chdir(pwd->pw_dir) < 0) {

		/*	No directory					*/
		printf(NODIR, pwd->pw_dir);
		goto Login;
	}

	/*	Set login time						*/
	time(&log_info.ut_time);

	/*	Get relative position for the tty-record in the		*/
	/*	'ttys' file which is the same as for the 'utmp' file	*/
	tty_fpos = ttyslot();

	/*	If the position could be evaluated, make an entry	*/
	/*	in the 'utmp' file that contains info about who		*/
	/*	is logged in at what terminal and the 'wtmp' file	*/
	/*	that contains info about all logins and logouts		*/
	if (tty_fpos >= 0) {
		if ((filedsc = open(UTMP_FILE, 1)) != -1) {

			/*	Position to the right entry		*/
			lseek(filedsc, (long)(tty_fpos * sizeof(log_info)), 0);

			/*	Write record				*/
			if (write(filedsc, (char *)&log_info, 
				sizeof(log_info)) != 
				sizeof(log_info))
				fprintf(stderr, WRERR, UTMP_FILE);
			close(filedsc);
		} else
			fprintf(stderr, NOOPEN, UTMP_FILE);

		if ((filedsc = open(WTMP_FILE, 1)) != -1) {

			/*	Position to the end of the file		*/
			lseek(filedsc, 0L, 2);

			/*	Write record				*/
			if (write(filedsc, (char *)&log_info, 
				sizeof(log_info)) != 
				sizeof(log_info))
				fprintf(stderr, WRERR, WTMP_FILE);
			close(filedsc);
		} else
;/*			fprintf(stderr, NOOPEN, WTMP_FILE);*/
	}

	/*	Set group and user name of the terminal			*/
	chown(tty_name, pwd->pw_uid, pwd->pw_gid);

	/*	Set UID and GID of this process				*/
	/*	Set UID and GID of this process				*/
	if (setgid(pwd->pw_gid) != 0) {
		fprintf(stderr, BADGNR, pwd->pw_gid);
		exit(2);
	}
	if (setuid(pwd->pw_uid) != 0) {
		fprintf(stderr, BADUID, pwd->pw_uid);
		exit(2);
	}

	/*	Initiate the shell name of the password record		*/
	if (*pwd->pw_shell == '\0')
		pwd->pw_shell = SHELL_FILE;

	/*	Make an environment for the shell process, and put it	*/
	/*	in the global string pointer 'environ'			*/
	environ = envmnt;

	/*	Initiate home path					*/
	if (strlen(pwd->pw_dir)+strlen(home) >= sizeof(home))
		fprintf(stderr, LNGNAM, pwd->pw_dir);
	strncat(home, pwd->pw_dir, sizeof(home)-strlen(home)-1);

	/*	Initiate shell name					*/
	strncat(sh_name, dname(pwd->pw_shell), sizeof(sh_name)-
		strlen(sh_name)-1);

	/*	Set file creation mask to 'rwxr-xr-x'			*/
	umask(022);

	/*	Show 'message of the day' -file				*/
	showmotd();

	/*	Test if there is any mail				*/
	if (strlen(pwd->pw_name)+strlen(MAIL_DIR)+1 >= sizeof(mail_dir))
		fprintf(stderr, LNGNAM, MAIL_DIR);
	else {
		strcat(mail_dir, SDELIM);
		strcat(mail_dir, pwd->pw_name);
		if (stat(mail_dir, &statblk) != -1 && statblk.st_size != 0)
			printf(MAILMSG);
	}

	/*	Set signals to default values				*/
	signal(SIGQUIT, SIG_DFL);
	signal(SIGINT, SIG_DFL);

	/*	Try to execute shell					*/
	execlp(pwd->pw_shell, sh_name, 0);

	/*	Could not execute shell					*/
	fprintf(stderr, NOEXEC, pwd->pw_shell);
	exit(0);
}

int	stopmotd;

catch()
{
	stopmotd++;
}

showmotd()

{
	int	lu;
	int	i;
	char	buf[512];

	signal(SIGINT, catch);
	if((lu = open(MOTD_FILE,0)) >= 0){
		while((i = read(lu,buf,sizeof buf)) > 0){
			write(fileno(stdout),buf,i);
		}
		close(lu);
	} else
		printf(NOOPEN, MOTD_FILE);
	signal(SIGINT, SIG_IGN);
}

sig_alrm()
{
	printf("\n");
	kill(getpid(), SIGALRM);
}
