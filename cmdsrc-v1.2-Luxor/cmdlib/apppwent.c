/*
 *	Routines for administrating the passwd file.
 *
 *	Copyright (C) DIAB T{by 1984
 *	Written by: Per-Arne Svensson 
 */

#include <stdio.h>
#include <signal.h>
#include <pwd.h>
#include <sys/types.h>
#include "../cmd.h"
#include "../cmd_err.h"


/*	Function for appending an entry to the passwd file appwent()	*/
/*	============================================================	*/
apppwent(pwd)
register struct	passwd *pwd;
{
int	putpwent(), res;
static	char	pwdfile[] = PASSWD_FILE;
register FILE	*pwd_str;
	/*	If the passwd file doesn't exist, create it		*/
	if (access(pwdfile, 0) == -1 && close(creat(pwdfile, 644)) == -1) {
		fprintf(stderr, NOCREA, pwdfile);
		return(-1);
	}

	/*	Test if the file can be opened for writing		*/
	/*	Protect the passwd file from being written by someone	*/
	if (mk_lock(PASSWD_LOCK, 240) == -1)
		return(-1);

	if ((pwd_str = fopen(pwdfile, "a")) == NULL) {
		fprintf(stderr, NOOPEN, pwdfile);
		rmv_lock(PASSWD_LOCK);
		return(-1);
	}

	res = putpwent(pwd, pwd_str);

	/*	Close passwd file					*/
	fclose(pwd_str);

	/*	Reset the passwd file status				*/
	rmv_lock(PASSWD_LOCK);

	return(res);
}

