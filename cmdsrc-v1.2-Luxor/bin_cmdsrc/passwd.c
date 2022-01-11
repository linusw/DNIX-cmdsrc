/*
 *	passwd command
 *
 *	Copyright (C) DIAB T{by 1983
 *	Written by: Per-Arne Svensson 
 */

/*
 *	passwd [name]
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <signal.h>
#include <pwd.h>
#include "../cmd.h"
#include "../cmd_err.h"

/*	External functions						*/
/*	==================						*/
struct passwd *getpwnam(), *getpwent();
char	*mk_pwd();		/*	Make a new encrypted passwd	*/
int	rmvpwent();		/*	Remove passwd entry		*/
int	apppwent();		/*	Append passwd entry		*/
int	setpwent();		/*	Reset and open password file	*/
int	endpwent();		/*	Close password file		*/
char	*getpass();		/*	Get passwd from term or file	*/
char	*getlogin();		/*	Get login name			*/
char	*malloc();		/*	Alocate memory			*/
int	strcmp();		/*	Compare strings			*/
int	strlen();		/*	Length of string		*/

/*	Global variables						*/
/*	================						*/

char	*newpwd();		/*	Get new password		*/
struct	passwd	*pwd;		/*	Password entry pointer		*/
struct	passwd	pwdent;		/*	Password entry			*/
char	newpass[16];		/* Place for new password		*/

/*	Main program							*/
/*	============							*/
main(argc, argv)
char *argv[];
int  argc;

{
char	bak_file[MAXN];		/*	Backup file name		*/
struct	entry	*bak_ptr;	/*	Backup file entry pointer	*/
FILE	*file;			/*	Input/Output stream		*/
char	*name_ptr, *pwd_ptr;	/*	Login/Password character pointer*/
				/*	Login/Password name		*/
int	getuid();		/*	UID number (0 if superuser)	*/
int	geteuid();		/*	UID number (0 if superuser)	*/
int	uid;			/*	UID number (0 if superuser)	*/
int	i, chr;			/*	Login character variable	*/

	/*	Initiate prompt string					*/
	PRMPT = *argv;

	if (geteuid() != 0) {	/*	No superuser-permission		*/
		fprintf(stderr, SUONLY);
		exit(1);
	}

	--argc;
	++argv;

	umask(0);

	/*	Initiate login-name					*/
	if (argc != 0)
		name_ptr =  *argv;
	else if ((name_ptr = getlogin()) == NULL) {
		printf("usage: %s [user]\n", PRMPT);
		exit(1);
	} else
		printf(CHGPWD, name_ptr);

	/*	Get passwd						*/
	if ((pwd = getpwnam(name_ptr)) == NULL || (uid = getuid()) != 0 &&
		uid != pwd->pw_uid) {
		printf(NOPERM);
		exit(1);
	}

	/*	Close password file					*/
	endpwent();

	/*	If there is any password and user is not		*/
	/*	superuser, encrypt it and check				*/

	if (*pwd->pw_passwd != '\0' && uid != 0 )
		if (strcmp(crypt(getpass(OLDPWD), pwd->pw_passwd), 
		pwd->pw_passwd) != 0) {
			printf(BADPWD);
			exit(1);
		}


	/*	Get a new password					*/
	if ((pwd_ptr = newpwd()) == NULL) {
		/*	Password unchanged				*/
		printf(UCHPWD);
		exit(1);
	}

	strcpy(newpass,pwd_ptr);
	pwd_ptr = newpass;

	if (strcmp(pwd_ptr, getpass(RTPPWD)) != 0) {
		printf(SORRY);
		exit(1);
	}

	/*	Make a new password entry				*/
#define	MKSTR(str1, str2)	strcpy(((str1)=malloc(strlen(str2)+1)),(str2))
	MKSTR(pwdent.pw_name, pwd->pw_name);
	pwdent.pw_passwd = mk_pwd(pwd_ptr);
	pwdent.pw_uid = pwd->pw_uid;
	pwdent.pw_gid = pwd->pw_gid;
	pwdent.pw_quota = pwd->pw_quota;
	MKSTR(pwdent.pw_comment, pwd->pw_comment);
	MKSTR(pwdent.pw_gecos, pwd->pw_gecos);
	MKSTR(pwdent.pw_dir, pwd->pw_dir);
	MKSTR(pwdent.pw_shell, pwd->pw_shell);


	/*	Ignore signals						*/
	signal(SIGHUP, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT, SIG_IGN);

	/*	Remove old passwd entry					*/
	if (rmvpwent(pwdent.pw_name))
		exit(1);

	/*	Append the new passwd entry				*/
	if (apppwent(&pwdent))
		exit(2);

	exit(0);
}



/*	Function for fetching and checking new password - newpwd()	*/
char *
newpwd()
{
char	*chr_ptr;	/*	Pointer to password			*/
char	*chr1_ptr;
int	pwd_len;	/*	Length of password			*/
int	chr;
int	lower_case;
int	upper_case;
int	numeric;
int	other;
int	insist = 2;

loop:
	lower_case = FALSE;
	upper_case = FALSE;
	numeric = FALSE;
	other = FALSE;

	chr_ptr = getpass(NEWPWD);
	if ((pwd_len = strlen(chr_ptr)) == 0)
		return(NULL);

	/*	Check that the password is enough unique to be safe	*/

	for (chr1_ptr=chr_ptr; chr = *chr1_ptr; chr1_ptr++) {
		if (islower(chr)) lower_case = TRUE;
		else if (isupper(chr)) upper_case = TRUE;
		else if(isdigit(chr)) numeric = TRUE;
		else other = TRUE;
	}

	if ((other && pwd_len>=4) ||
		(lower_case || upper_case) && (pwd_len>=6 || 
		(lower_case && upper_case || numeric) && pwd_len>=5))
			return(chr_ptr);

	else if (insist--) {
		if (numeric && !lower_case && !upper_case && !other)
			/*	Please use at least one non_numeric char*/
			printf(PASSWD1);
		else
			/*	Please use a longer password		*/
			printf(PASSWD2);

		goto loop;
	}
	else return(chr_ptr);
}
