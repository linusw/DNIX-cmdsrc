/*
 *	Routines for administrating the password file.
 *
 *	Copyright (C) DIAB T{by 1984
 *	Written by: Per-Arne Svensson 
 */

#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>

/*	Function for writing an entry to the password file putpwent()	*/
/*	=============================================================	*/
putpwent(pwd, str)
register struct passwd *pwd;
register FILE	*str;
{
	fprintf(str, "%s:%s:%d:%d:%s:%s:%s\n",
	pwd->pw_name,				/*	User name	*/
	pwd->pw_passwd,				/*	User password	*/
	pwd->pw_uid,				/*	User ID		*/
	pwd->pw_gid,				/*	Group ID	*/
	pwd->pw_gecos,
	pwd->pw_dir,				/*	Directory	*/
	pwd->pw_shell);				/*	Command		*/
}
