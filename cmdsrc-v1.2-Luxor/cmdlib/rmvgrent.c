/*
 *	Routines for administrating the group file.
 *
 *	Copyright (C) DIAB T{by 1984
 *	Written by: Per-Arne Svensson 
 */

#include <stdio.h>
#include <signal.h>
#include <grp.h>
#include <sys/types.h>
#include "../cmd.h"
#include "../cmd_err.h"

/*	Function for removing a group from the group file - rmvgrent()	*/
/*	==============================================================	*/
rmvgrent(name)
register char *name;		/*	Group name in file		*/
{
struct	group *grp, *getgrent();/*	Group record 			*/
struct	entry *makentry();	/*	Make one tree entry		*/
struct	entry *freetree();	/*	Free memory occupied by tree	*/
struct entry *bak_ptr, *grp_ptr, *tmp_ptr;
register FILE	*out_str;
static	char grpfile[] = GROUP_FILE;	/*	Group file		*/
static	char tmp_grp[] = TEMP_FILE;	/*	Temporary group file	*/
static	char bak_file[MAXN];		/*	Backup file		*/

	/*	Test if the group file can be opened for reading	*/
	/*	Protect the group file from being written by someone	*/
	if (mk_lock(GROUP_LOCK, 240) == -1)
		return(-1);

	/*	Open temporary group file			*/
	if (access(mktemp(tmp_grp), 0) != -1) {
		fprintf(stderr, FEXIST, tmp_grp);
		rmv_lock(GROUP_LOCK);
		return(-1);
	}

	if (close(creat(tmp_grp, 644)) == -1 || 
		(out_str = fopen(tmp_grp, "w")) == NULL) {
		fprintf(stderr, NOCREA, tmp_grp);
		rmv_lock(GROUP_LOCK);
		return(-1);
	}

	/*	Reset group file					*/
	setgrent();

	while ((grp = getgrent()) != NULL)
		if (strcmp(grp->gr_name, name))
			putgrent(grp, out_str);


	/*	Close group file					*/
	endgrent();

	fclose(out_str);

	/*	Link old password file to backup file			*/
	strcpy(bak_file,grpfile);
	strcat(bak_file,".bak");

	if ((bak_ptr = makentry(bak_file,"")) != NULL)
		if (link(grpfile, bak_ptr->e_fname) == 0)
			unlink(grpfile);

	/*	Try to link or copy temporary file			*/
	/*	to group file						*/
	if (rgcopy((tmp_ptr = makentry(tmp_grp, "")), 
		(grp_ptr = makentry(grpfile, "")), FALSE, TRUE) != 0)

		/*	Restore group file				*/
		if (bak_ptr != NULL)
			link(bak_ptr->e_fname, grpfile);

	freetree(bak_ptr);
	freetree(tmp_ptr);
	freetree(grp_ptr);


	/*	Remove temporary file					*/
	if (unlink(tmp_grp) < 0) {
		fprintf(stderr, NOUNLN, tmp_grp);
		rmv_lock(GROUP_LOCK);
		return(-1);
	}

	/*	Reset the group file status				*/
	rmv_lock(GROUP_FILE);

	return(0);
}
