/*
 *	du - summarize disk usage
 *
 *	Copyright (C) DIAB T{by 1984
 *	Written by: Per-Arne Svensson 
 */

/*
 *	du [ -afsu ] [ name ... ]
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include "../cmd.h"
#include "../cmd_err.h"

/*	External functions						*/
/*	==================						*/
int	setbuf();
char	*strcpy(), *strncpy();	/*	Copy a string			*/
char	*strcat(), *strncat();	/*	Concatenate strings		*/
int	strcmp();		/*	Compare strings			*/
int	strlen();		/*	Length of string		*/
char	*pname();		/*	Get path-name of file		*/
char	*dname();		/*	Get directory-name of file	*/
struct	entry *makentry();	/*	Make one tree entry		*/
struct	entry *freetree();	/*	Free memory occupied by tree	*/
struct	stat  *getstat();	/*	Make one status block entry	*/
instree();			/*	Insert entry in tree structure	*/
fileinfo();			/*	Print info about a file		*/

/*	Stream-I/O buffers						*/
/*	==================						*/
char	dirbuf[BUFSIZ];			/*	Directory read buffer	*/

/*	Global variables						*/
/*	================						*/

int	aflg=FALSE;		/*	List all files			*/
int	fflg=FALSE;		/*	Ignore mounted filesystems	*/
int	sflg=FALSE;		/*	Summarize usage only		*/
int	uflg=FALSE;		/*	Count onlu files with one link	*/

listtree();			/*	List tree structure		*/
off_t	du_dir();		/*	Disk usage, all directories	*/
off_t	dir_du();		/*	Disk usage one directory 	*/
long	compare();		/*	Compare two entrys		*/
off_t	total();		/*	Total size of directory		*/
struct	entry	*linktree;	/*	Tree of files with several links*/

/*	Main program							*/
/*	============							*/
main(argc, argv)
char *argv[];
int  argc;

{
	int i;
	off_t	size;			/*	Total size in blocks	*/
	struct	entry *root;		/*	Root of du-tree		*/
	struct	entry *entryptr;

	/*	Initiate prompt string					*/
	PRMPT = *argv;

	/*	Assign stream-I/O buffer to input (directory) stream	*/
	setbuf(stdin, dirbuf);

	/*	Set real UID and effective UID to real UID		*/
	setuid(getuid());

	/*	Test if any switches					*/
	--argc;
	++argv;
	while (*argv && **argv == '-') {
		while (*++*argv) switch (**argv) {
		case 'a':		/*	List all files		*/
			aflg = TRUE;
			continue;
		case 'f':		/*	Ignore mounted filesyst	*/
			fflg = TRUE;
			continue;
		case 's':		/*	List summary info	*/
			sflg = TRUE;
			continue;
		case 'u':		/*	Files with one link only*/
			uflg = TRUE;
			continue;
		default:
			fprintf(stderr, BADSW, *argv);
			argc = -1;
		}
		--argc;
		++argv;
	}

	if (argc < 0) {
		fprintf(stderr,"usage: %s du [ -afsu ] [ name ... ]\n", PRMPT);
		return(-1);
	}

	if (sflg) {
		aflg = FALSE;
	}

	if (argc != 0) {
		root = makentry(pname(*argv), dname(*argv));
		
		for (i = 1; i < argc; ++i)
			instree(makentry(pname(argv[i]),dname(argv[i])), root);

		listtree(root);

		size = du_dir(root);
		freetree(root);

	} else {
		root = makentry("", DOT);
		size = dir_du(root);
		freetree(root);
	}
	printf("%s: %ld bytes\n", 
		PRMPT, size);
	return(0);
}



/*	Function for comparing two records according to inode number	*/
/*	============================================================	*/
long
compare(ptr1, ptr2)
register struct entry *ptr1, *ptr2;
{
long	result;
	if (getstat(ptr1) == NULL || getstat(ptr2) == NULL)
		result = 0;
	else
		result = (long)(ptr2->e_stat->st_ino - ptr1->e_stat->st_ino);
	return(result);
}

/*	Function for recursive disk usage of one directory		*/
/*	==================================================		*/
off_t
dir_du(ptr)
register struct entry *ptr;
{
register struct direct	dir_entry;	/*	Directory entry		*/
register struct entry	*dir_tree = NULL;/*	Root of directory tree	*/
off_t	result = 0;			/*	Byte count of the tree	*/

	if (ptr == NULL || getstat(ptr) == NULL)
			return(0);
	/*	If the directory is a mounted filesystem		*/
	/*	and fflg is set, ignore it and return zero		*/
	if (fflg && ptr->e_stat->st_ino == ROOTINO && strcmp(ptr->e_fname, SDELIM))
		return(0);

	if (freopen(ptr->e_fname, "r", stdin) == NULL) {
		fprintf(stderr, NOOPEN, ptr->e_fname);
		return(0);
	}

	/*	Make a new tree and list it				*/


	while (fread((char *)&dir_entry, sizeof(dir_entry), 
		1, stdin) == 1) {

		/*	Test if the file is deleted			*/
		/*	or if it should not be counted			*/
		if (dir_entry.d_ino == 0 ||
			!strcmp(dir_entry.d_name, DOT) ||
			!strcmp(dir_entry.d_name, DOTDOT))
			continue;

		if (dir_tree == NULL)
			dir_tree = makentry(ptr->e_fname, dir_entry.d_name);
		else
			instree(makentry(ptr->e_fname, dir_entry.d_name), dir_tree);
	}

/*	fclose(stdin); should not be used with freopen()		*/

	/*	Recurse to see if any directories here			*/
	result = total(dir_tree) + du_dir(dir_tree);
	if (!sflg)
		printf("\n%s:\ntotal:%ld bytes\n", ptr->e_fname, result);

	/*	List the directory					*/
	if (aflg)
		listtree(dir_tree);

	/*	Free dynamically allocated space			*/
	freetree(dir_tree);
	dir_tree = NULL;
	return(result);
}

/*	Function for recursive disk usage of the directories		*/
/*	====================================================		*/
off_t
du_dir(ptr)
register struct entry *ptr;
{
	off_t	result;
	if (ptr == NULL)
			return(0);

	result = du_dir(ptr->left);
	if ((strcmp(dname(ptr->e_fname), DOT) && 
		strcmp(dname(ptr->e_fname), DOTDOT))) {

		/*	Test if the entry ptr-> is a directory		*/
		if (getstat(ptr) != NULL && isdir(*ptr->e_stat))
			result += dir_du(ptr);
	}

	result += du_dir(ptr->right);
	return(result);
}

/*	Function for recursive listing of the binary file-list tree	*/
/*	===========================================================	*/
listtree(ptr)
register struct entry *ptr;
{
	if (ptr == NULL)
			return(0);
	listtree(ptr->left);

	if (getstat(ptr) != NULL)
		fileinfo(ptr->e_fname, ptr->e_stat, 
			TRUE,TRUE,TRUE,FALSE,FALSE,FALSE);
	else
		fprintf(stderr, NOACC, ptr->e_fname);

	listtree(ptr->right);
	return(0);
}

/*	Function returning total file-space occupied by a directory	*/
/*	========================================================	*/
off_t
total(ptr)
register struct entry *ptr;
{
off_t size = 0;
register unsigned short ftype;
register short linkcnt;
struct	entry	*entry;
	if (ptr == NULL || getstat(ptr) == NULL)
		return(0L);

	if ((linkcnt = ptr->e_stat->st_nlink) == 1 && uflg || 
		!counted(ptr->e_stat, linktree)) {
		
		/*	If the file has more than one link to it	*/
		/*	insert it into the linktree			*/

		if (linkcnt > 1) {
			if (linktree == NULL)
				linktree = (entry = makentry(ptr->e_fname, ""));
			else
				instree((entry = makentry(ptr->e_fname, "")),linktree);
		}

		/*	Get the size of the file			*/

		if ((ftype = ptr->e_stat->st_mode & S_IFMT) == S_IFCHR
						|| ftype == S_IFBLK)
			size = ptr->e_stat->st_rdev;
		else
			size = ptr->e_stat->st_size;
	}
	return(total(ptr->left)+total(ptr->right) + size);
}

/*	Function testing if a file with more than one link is counted	*/
/*	=============================================================	*/
counted(stat, ptr)
register struct stat *stat;
register struct entry *ptr;
{
	if (ptr == NULL || getstat(ptr) == NULL)
		return(0);

	if (ptr->e_stat->st_ino == stat->st_ino && 
		ptr->e_stat->st_dev == stat->st_dev)
		return(1);
	else
		return(counted(stat, ptr->left)+counted(stat, ptr->right));
}
