/*
 *	su command
 *
 *	Copyright (C) DIAB T{by 1983
 *	Written by: Per-Arne Svensson 
 */

/*
 *	su [name]
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <pwd.h>
#include "../cmd.h"
#include "../cmd_err.h"

/*	External functions						*/
/*	==================						*/
struct passwd *getpwnam();
int	setpwent();		/*	Reset and open password file	*/
int	endpwent();		/*	Close password file		*/
char	*crypt();		/*	Encryption routine		*/
char	*getpass();		/*	Get passwd from term or file	*/
char	*strcpy(), *strncpy();	/*	Copy a string			*/
char	*strcat(), *strncat();	/*	Concatenate strings		*/
int	strcmp();		/*	Compare strings			*/
int	strlen();		/*	Length of string		*/

/*	Global variables						*/
/*	================						*/

struct	passwd	nouser	= {"", "nope"};	/*	No_user password entry	*/
struct	passwd	*pwd;			/*	Password pointer	*/
char	su[DIRSIZ] = "su";		/*	Superuser shell name	*/
int	eflg=FALSE;			/*	Login environment	*/
char	**envptr;			/*	Global environment ptr	*/


/*		Environment definition					*/
char	path[]		= "PATH=:/bin:/usr/bin";
char	supath[]	= "PATH=:/bin:/etc:/usr/bin";
char	home[MAXN+5]	= "HOME=";
char	*envmnt[]	= {home, path, 0};
extern	char	**environ;		/*	External environment ptr*/


/*	Main program							*/
/*	============							*/
main(argc, argv)
char *argv[];
int  argc;

{
char	*name_ptr;		/*	Pointer to login name		*/
register int filedsc;		/*	File descriptor			*/

	/*	Initiate prompt string					*/
	PRMPT = *argv;

	if (geteuid() != 0) {	/*	No superuser-permission		*/
		fprintf(stderr, SUONLY);
		exit(1);
	}

	--argc;
	++argv;

	/*	Close all files except stdin, stdout & stderr		*/
	for (filedsc = 0; filedsc < _NFILE; ++filedsc)
		if (filedsc != fileno(stdin) && 
			filedsc != fileno(stdout) && 
			filedsc != fileno(stderr))
			close(filedsc);

	/*	Test if login environment should be used		*/
	if (*argv && **argv == '-') {
		eflg = TRUE;
		--argc;
		++argv;
	}

	/*	Initiate login-name					*/
	if (*argv) {
		name_ptr =  *argv;
		--argc;
		++argv;
	} else
		name_ptr = ROOT_NAME;	/*	Name of root directory	*/

	/*	Get passwd						*/
	if ((pwd = getpwnam(name_ptr)) == NULL) {
		fprintf(stderr, BADUID, name_ptr);
		exit(1);
	}

	/*	Close password file					*/
	endpwent();

	/*	If there is any password and the user is not superuser	*/
	/*	, encrypt it and check					*/
	if (*pwd->pw_passwd != '\0' && getuid() != 0) {
		name_ptr = crypt(getpass(PASSWD), pwd->pw_passwd);

		/*	If not correct abort process			*/
		if (strcmp(name_ptr, pwd->pw_passwd) != 0) {
			printf(SORRY);
			exit(2);
		}
	}

	/*	Set UID and GID of this process				*/
	if (setgid(pwd->pw_gid) != 0) {
		fprintf(stderr, BADGNR, pwd->pw_gid);
		exit(2);
	}
	if (setuid(pwd->pw_uid) != 0) {
		fprintf(stderr, BADUID, pwd->pw_uid);
		exit(2);
	}

	/*	If change of environment (eflg)				*/
	if (eflg) {
		strcat(home, pwd->pw_dir);
		if (chdir(pwd->pw_dir) < 0) {
		/*	No directory					*/
			printf(NODIR, pwd->pw_dir);
			exit(2);
		}
		if (pwd->pw_uid == 0)
			envmnt[1] = supath;
		else
			envmnt[1] = path;
		strcpy(su, "-su");
		environ = envmnt;
	}

	/*	If superuser and no change of environment		*/
	if (pwd->pw_uid == 0 && !eflg) {

		/*	Modify the environment				*/
		for (envptr = environ; *envptr ; envptr++)
			if (strncmp("PS1=", *envptr, 4) == 0) {
				*envptr = "PS1=# ";
				break;
			}
		for (envptr = environ; *envptr ; envptr++)
			if (strncmp("PATH=", *envptr, 5) == 0) {
				*envptr = supath;
				break;
			}
	}

	/*	If any arguments are given				*/
	if (*argv) {

		--argv;
		++argc;
		*argv = su;

		/*	Try to execute shell				*/
		execv(SHELL_FILE, argv);
	} else {

		/*	Initiate the shell name of the password record	*/
		if (*pwd->pw_shell == '\0')
			pwd->pw_shell = SHELL_FILE;
		else {
			strcpy(su, eflg ? "-" : "");
			strcat(su, pname(pwd->pw_shell));
		}

		/*	Try to execute shell				*/
		execl(pwd->pw_shell, su,  0);
	}

	/*	Could not execute shell					*/
	fprintf(stderr, NOEXEC, pwd->pw_shell);
	exit(3);
}
