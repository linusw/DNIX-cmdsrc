/*
 * 	mv - move command
 *
 *	Copyright (C) DIAB T{by 1983
 *	Written by: Per-Arne Svensson 
 */

/*
 * mv f1 f2; or mv d1 d2; or mv f1 ... fn d1;
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <signal.h>
#include "../cmd.h"	/*	Command include-file			*/
#include "../cmd_err.h"	/*	Error-messages				*/

			/*	External declarations			*/
			/*	=====================			*/

char	*pname();	/*	Function returning pathname	*/
char	*dname();	/*	Function returning dirname	*/
char	*filemode();	/*	Function returning file-mode	*/


main(argc, argv)
char	*argv[];	/*	Argument pointer		*/
int	argc;		/*	Argument count			*/

{
struct	stat	s1,s2;	/*	File status			*/
	register i, r;	/*	Loop variables			*/

	/*	Initiate prompt string					*/
	PRMPT = *argv;

	if (geteuid() != 0) {	/*	No superuser-permission		*/
		fprintf(stderr, SUONLY);
		exit(1);
	}

	/*	Set file-protection mask			*/
	/*	========================			*/
	umask(0);

	/*	Check if number of arguments is too small	*/
	/*	=========================================	*/
	if	(argc < 3 )	goto usage;

	/*	Test if first file is accessible		*/
	/*	================================		*/
	if	(stat(argv[1], &s1) < 0)
	{
		fprintf(stderr, NOACC, argv[1]);
		return(1);
	}

	/*	Test if first file is a directory		*/
	/*	=================================		*/
	if	isdir(s1)
	{

		/*						*/
		/*	Then only two files can be specified	*/
		/*						*/
		if	( argc != 3 ) goto usage;

		/*	Move directories			*/
		/*	================			*/
		return	mvdir( argv[1],argv[2] );
	}

	/*	Set real UID and effective UID to real UID	*/
	/*	==========================================	*/
	setuid(getuid());

	/*							*/
	/*	If more than one file is moved,			*/
	/*	the target must be a directory			*/
	/*							*/
	if	((argc > 3 ) && (stat(argv[argc-1],&s2) < 0 || !isdir(s2)))
		goto usage;

	/*	Loop and move the files				*/
	/*	=======================				*/
	r = i = 0;

	while	( ++i < argc-1 )
		r |= move( argv[i],argv[argc-1] );
	return(r);

	/*							*/
	/*	User description of move-command		*/
	/*							*/
usage:
	fprintf(stderr,"usage: %s f1 f2; || mv d1 d2; || mv f1 ... fn d1 \n", PRMPT);
	return(1);
}

move(source,target)	/* move source-file to target-file	*/
char	*source,*target;	/*	Parameters			*/

{
struct	stat		s1,s2;	/*	File status			*/
	register	c,i;
	int		status;
	char		buf[MAXN];

	/* Test if source-file is accessible	*/

	if	(stat(source, &s1) < 0)
	{
		fprintf(stderr, NOACC, source);
		return(1);
	}

	/* Test if source-file is a directory	*/

	if	isdir(s1)
	{
		fprintf(stderr, FISDIR,target);	/* Cant move more direcories */
		return(1);
	}

	/* Check that the target-file exists	*/

	if	(stat(target, &s2) >= 0 )
	{

		/* If target-file is a directory, then add the source-file name  */

		if	isdir(s2)
		{
			strcpy(buf,target);
			strcat(buf,"/");
			strcat(buf,dname(source));
			target = buf;
		}

		/* Test if the new target file already exists */

		if	(stat(target, &s2) >= 0 )
		{
			if	isdir(s2)
			{

				/* The target-file is a directory */
				fprintf(stderr, FISDIR, target);
				return(1);
			}

			/* Test if files are identical */
			if (s1.st_dev == s2.st_dev && s1.st_ino == s2.st_ino)
			{
				fprintf(stderr, FIDENT, source, target);
				return(1);
			}

			/* If the target-file is write-protected and if the
					 	* standard-input is the tty, promt with the mode-bits
					 	* and return if not a 'y' is answered. 
			*/

			if (access(target, 2) < 0 && isatty(fileno(stdin)))
			{
				fprintf(stderr, MODE,target,
				filemode(&s2));
				if (ask(TRUE, OVRMSG) == FALSE)  return(1);
			}

			/* Try to unlink the target file	*/
			if	(unlink(target) < 0)
			{
				/* Cannot unlink target-file	*/
				fprintf(stderr, NOUNLN, target);
				return(1);
			}
		}
	}

	/* Try to link the target-file to the source-file	*/
	if	(link(source, target) < 0)

		/* It did not succeed, probably because the files belongs
			 * to different devices, so try to make a copy instead by
			 * executing the process "cp" by using fork and exec	*/
	{
		/* Create a child-process		*/
		i	= fork();

		/* Test if creation succeeded		*/
		if	(i == -1)
		{
			/* Not enough memory or swap-space	*/
			fprintf(stderr, AGAIN);	/* Try again	*/
			return(1);
		}

		/* Test if we are in the child-process	*/
		if	(i == 0)
		{
			execlp("cp","cp", source, target, 0);
			/* It did obviously not succeed	*/
			fprintf(stderr, NOEXEC, "\"cp\"");
			exit(1);		/* Terminate child	*/
		}

		/* Now we are in the parent-process: 			*/
		/* wait for the child to terminate 			*/
		/* and test if the result is OK				*/
		while	((c = wait(&status)) != i && c != -1);

		/* Test if the child-process terminated abnormally	*/
		if	(status != 0)
		{
			/* Failed to move source to target		*/
			fprintf(stderr, FAILMV, source, target);
			return(1);
		}

		/* Set file-time for target-file	*/
		utime(target, &s1.st_atime);
	}

	/* Try to unlink the source-file	*/
	if (unlink(source) < 0)
	{
		/* Cannot unlink source-file	*/
		fprintf(stderr, NOUNLN, source);
		return(1);
	}

	/* OK move complete	*/
	return(0);
}


mvdir(source, target)		/*	move directories	*/
char	*source,*target;

{
struct	stat		s1,s2;	/*	File status		*/
	register	char	*p;
	register	i;
	char		buf[MAXN];

	/* Test if source-file is accessible		*/

	if	( stat(source, &s1) < 0)
	{
		fprintf(stderr, NOACC, source);
		return(1);
	}

	/* Test if target-file is accessible		*/
	if	(stat(target, &s2) >= 0)
	{
		/* Test if target-file is a directory	*/
		if	(!isdir(s2))
		{
			/* Target-file exists but is not a directory	*/
			fprintf(stderr, NODIR, target);
			return(1);
		}

		/* Check that there is enough place for the file-names	*/
		if	(strlen(target) > MAXN-DIRSIZ-2)	{
			fprintf(stderr, LNGNAM, target);
			return(1);
		}

		/* Add source directory-name to target-path-name	*/
		strcpy(buf,target);
		target = buf;
		strcat(target, SDELIM);
		strcat(target, dname(source));

		/*		 Test if target-file is accessible		*/
		if	(stat(target, &s2) >= 0)	{
			/* Target directory already exists		*/
			fprintf(stderr, FEXIST, target);
			/* Ask if override	*/
			if	(!overrd())	return(1);
		}
	}

	/* Test if source and target are identical	*/
	if	(strcmp(source,target) == 0)	{
		/* Files are identical	*/
		fprintf(stderr, FILIDN);
		return(1);
	}

	/* Test if directory-name is acceptable		*/

	p = dname(source);
	if (!strcmp(p, DOT) || !strcmp(p, DOTDOT) || !strcmp(p, "")
	    || p[strlen(p)-1] == '/')	{
		fprintf(stderr, NORENM, p);
		return(1);
	}

	/* Test source & Target path-names		*/
	if (stat(pname(source), &s1) < 0) {
		/* Cannot locate parent	*/
		fprintf(stderr, NOPRNT, source);
		return(1);
	}
	if (stat(pname(target), &s2) < 0) {
		/* Cannot locate parent	*/
		fprintf(stderr, NOPRNT, target);
		return(1);
	}

	/* Test if target-path has write-access	*/
	if (access(pname(target), 2) < 0) {
		/* No write-access to target path-name	*/
		fprintf(stderr, NWRACC, pname(target));
		return(1);
	}

	/* Test if source-path has write-access	*/
	if (access(pname(source), 2) < 0) {
		/* No write-access to source path-name	*/
		fprintf(stderr, NWRACC, pname(source));
		return(1);
	}

	/* Test if the paths lies on different devices	*/

	if (s1.st_dev != s2.st_dev) {
		/* Can't move directories between devices	*/
		fprintf(stderr, NBTDEV);
		return(1);
	}

	/*
		 * If the directories to move, has different parents, then both
		 * the directories and their parent directories (..) has to be
		 * moved. First move the directory, then unlink the parent-
		 * directory and link the directorys new parent to it.
		 */
	/* Test if the directories has different parents	*/
	if (s1.st_ino != s2.st_ino) {

			/*
			 * Declare a string with enough place to contain the file-name
			 * including the parent directory (..).
			 */
		char	dst[MAXN+5];

		/* Test if the source or target contains (..)	*/
		if (chkdot(source) || chkdot(target)) {
			fprintf(stderr, NOPDIR, DOTDOT);
			return(0);

		}

		/*
				 * Check that the target-path is accessible and doesn't contain
				 * the source-directory inode (not a tree-structure)
				 */
		if (check(pname(target), source))	return(1);

		/* Ignore all signals	*/
		for (i = 0; i <= NSIG; i++) signal(i, SIG_IGN);

		/* Link source-directory to target-directory	*/
		if (link(source, target) < 0) {
			/* Cannot link source to target	*/
			fprintf(stderr, NOLINK, source, target);
			return(1);
		}

		/* Unlink source-directory	*/
		if (unlink(source) < 0) {
			fprintf(stderr, NOUNLN, source);
			/* Unlink target-directory to undo the operation	*/
			unlink(target);
			return(1);
		}

		/* Update the parent-directory of the target	*/
		/* Let dst contain the parent-directory-name of the target	*/
		strcpy(dst, target);
		strcat(dst, SDELIM);
		strcat(dst, DOTDOT);

		/* Unlink the previous parent-directory	*/
		if (unlink(dst) < 0) {
			/* Cannot unlink parent-directory of target	*/
			fprintf(stderr, NOUNLN, dst);
			/* Undo the move operation by linking back to source again	*/
			if (link(target, source) >= 0) unlink(target);
			return(1);
		}

		/* Link parent to targets parent-directory		*/
		if (link(pname(target), dst) < 0) {
			/* Cannot link parent to (..)	*/
			fprintf(stderr, NOLINK, pname(target), dst);
			/* Undo the previous unlink of the parent-diretory (..)	*/
			if (link(pname(source), dst) >= 0) {
				if(link(target, source) >= 0) unlink(target);
				return(1);
			}

		}

		/* Operation complete, no errors	*/
		return(0);

	}

	/*
		 * The source-directory and the target-directory has the same
		 * parent so link source to target and unlink source
		 */

	if (link(source, target) < 0) {
		fprintf(stderr, NOLINK, source, target);
		return(1);
	}
	if (unlink(source) < 0) {
		fprintf(stderr, NOUNLN, source);
		unlink(target);
		return(1);
	}

	/* Operation complete, no errors	*/
	return(0);
}

/*
 * check - function checking the path for accessibility and
 * hierarchial concistency supposing file was appended to it
 */


check(path, file)
char	*path, *file;
{
	char	srcpth[MAXN];	/* Search path		*/
	struct	stat status;	/* File status		*/
	ino_t	dinode;			/* Directory inode	*/
	stat(file, &status);	/* Put file:s inode number in dinode	*/
	dinode = status.st_ino;
	status.st_ino = 0;		/* Initiate the inode number to zero	*/
	strcpy(srcpth, path);	/* Make a copy of the input path-name	*/

	/* do test directories in srcpth until root directory	*/
	do {
		/* Test if the path-directory is acessible	*/
		if (stat(srcpth, &status) < 0){
			/* Cannot access directory	*/
			fprintf(stderr, NOACC, srcpth);
			return(1);
		}
		/* Test if the path-directory and the new directory are equal	*/
		if (status.st_ino == dinode) {
			/*
					 * You cannot move something further down in a tree that
					 * lies on the same branch, because then the branch
					 * gets 'cut off' the tree.
					 */
			fprintf(stderr, SMPATH, path, file);
			return(1);
		}
		/* Test if one more DOTDOT can be merged to srcpth	*/
		if (strlen(srcpth) > MAXN-2-sizeof(DOTDOT)) {
			/* File-name too long	*/
			fprintf(stderr, LNGNAM, path);
			return(1);
		}
		/* Add DOTDOT to srcpth for acessing parent-directory	*/
		strcat(srcpth, SDELIM);
		strcat(srcpth, DOTDOT);
	}	
	while (status.st_ino != ROOTINO);
	return(0);
}

/* chkdot - check path-name for DOTDOT:s	*/

chkdot(path)
register char *path;
{
	do {
		if (strcmp(dname(path), DOTDOT) == 0) return(1);
		path = pname(path);
	} 
	while (strlen(path) != 0 && strcmp(path, SDELIM) != 0);
	return(0);
}

/* overrd - function determining whether override warning, or not	*/
overrd()
{
	register int i,c;
	fprintf(stderr, OVRMSG);
	i = c = getchar();
	while((c=getchar()) != '\n' && c != EOF );
	if	(i != 'y' && i != 'Y' )
	    return(0);
	else
		return(1);
}
