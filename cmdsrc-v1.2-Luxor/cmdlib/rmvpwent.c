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

/*	Function for removing a passwd from the passwd file - rmvpwent()*/
/*	==============================================================	*/
rmvpwent(name)
register char *name;		/*	Passwd name in file		*/
{
struct	passwd *pwd, *getpwent();/*	Passwd record 			*/
struct	entry *makentry();	/*	Make one tree entry		*/
struct	entry *freetree();	/*	Free memory occupied by tree	*/
struct entry *bak_ptr, *pwd_ptr, *tmp_ptr;
static	char pwdfile[] = PASSWD_FILE;	/*	Passwd file		*/
static	char tmp_pwd[] = TEMP_FILE;	/*	Temporary passwd file	*/
static	char bak_file[MAXN];		/*	Backup file		*/
register FILE	*out_str;

	/*	Test if the passwd file can be opened for reading	*/
	/*	Protect the passwd file from being written by someone	*/
	if (mk_lock(PASSWD_LOCK, 240) == -1)
		return(-1);

	/*	Open temporary passwd file			*/
	if (access(mktemp(tmp_pwd), 0) != -1) {
		fprintf(stderr, FEXIST, tmp_pwd);
		rmv_lock(PASSWD_LOCK);
		return(-1);
	}

	if (close(creat(tmp_pwd, 0644)) == -1 || 
		(out_str = fopen(tmp_pwd, "w")) == NULL) {
		fprintf(stderr, NOCREA, tmp_pwd);
		rmv_lock(PASSWD_LOCK);
		return(-1);
	}

	/*	Reset passwd file					*/
	setpwent();

	while ((pwd = getpwent()) != NULL)
		if (strcmp(pwd->pw_name, name))
			putpwent(pwd, out_str);

	/*	Close passwd file					*/
	endpwent();

	fclose(out_str);

	/*	Link old password file to backup file			*/
	strcpy(bak_file,pwdfile);
	strcat(bak_file,".bak");

	if ((bak_ptr = makentry(bak_file,"")) != NULL)
		if (link(pwdfile, bak_ptr->e_fname) == 0)
			unlink(pwdfile);

	/*	Try to link or copy temporary file			*/
	/*	to passwd file						*/
	if (rgcopy((tmp_ptr = makentry(tmp_pwd, "")), 
		(pwd_ptr = makentry(pwdfile, "")), FALSE, TRUE) != 0)

		/*	Restore passwd file				*/
		if (bak_ptr != NULL)
			link(bak_ptr->e_fname, pwdfile);

	freetree(bak_ptr);
	freetree(tmp_ptr);
	freetree(pwd_ptr);


	/*	Remove temporary file					*/
	if (unlink(tmp_pwd) < 0) {
		fprintf(stderr, NOUNLN, tmp_pwd);
		rmv_lock(PASSWD_LOCK);
		return(-1);
	}

	/*	Reset the passwd file status				*/
	rmv_lock(PASSWD_LOCK);

	return(0);
}
