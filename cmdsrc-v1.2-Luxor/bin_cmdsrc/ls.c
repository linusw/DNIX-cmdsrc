/*
 *	ls - list command
 *
 *	Copyright (C) DIAB T{by 1983
 *	Written by: Per-Arne Svensson 
 */

/*
 *	list files
 */

/*
 *	ls [ -ltasdrucifg ] name ...
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
char	lsbuf[BUFSIZ];			/*	Output buffer		*/
char	dirbuf[BUFSIZ];			/*	Directory read buffer	*/

/*	Global variables						*/
/*	================						*/

int	aflg=FALSE;		/*	List all files			*/
int	cflg=FALSE;		/*	Use file creation time		*/
int	dflg=FALSE;		/*	Only the names of dir's		*/
int	eflg=FALSE;		/*	List total tree structure	*/
int	fflg=FALSE;		/*	Force file to be dir		*/
int	gflg=FALSE;		/*	Group ID istead if User		*/
int	iflg=FALSE;		/*	List inode numbers		*/
int	lflg=FALSE;		/*	Long listing			*/
int	rflg=FALSE;		/*	Reverse sort			*/
int	sflg=FALSE;		/*	List filesize in blocks		*/
int	tflg=FALSE;		/*	Sort by time (def:name)		*/
int	uflg=FALSE;		/*	Use time of last access		*/
int	totf=FALSE;		/*	Write name and total size	*/
int	top_lev=FALSE;		/*	Indicate top directory level	*/

listtree();			/*	List tree structure		*/
listdir();			/*	List all directories		*/
dirlist();			/*	List one directory (recursive)	*/
long	compare();		/*	Compare two entrys		*/
off_t	total();		/*	Total size of directory		*/

/*	Main program							*/
/*	============							*/
main(argc, argv)
char *argv[];
int  argc;

{
	int i;
	struct	entry *root;		/*	Root of list-tree	*/
	struct	entry *entryptr;

	/*	Initiate prompt string					*/
	PRMPT = *argv;

	/*	Assign stream-I/O buffer to output stream		*/
	setbuf(stdout, lsbuf);

	/*	Assign stream-I/O buffer to input (directory) stream	*/
	setbuf(stdin, dirbuf);

	/*	Set real UID and effective UID to real UID		*/
	setuid(getuid());

	/*	If invoked under the name l, set appropriate switches	*/
	if (!strcmp(dname(*argv), "l"))
		lflg = TRUE;

	/*	Test if any switches					*/
	--argc;
	++argv;
	while (*argv && **argv == '-') {
		while (*++*argv) switch (**argv) {
		case 'a':		/*	List all files		*/
			aflg = TRUE;
			continue;
		case 'c':		/*	Use  ino mod-time	*/
			cflg = TRUE;
			continue;
		case 'd':		/*	Only the names of dir's	*/
			dflg = TRUE;
			continue;
		case 'e':		/*	List total tree structure*/
			eflg = TRUE;
			continue;
		case 'f':		/*	Force file to be dir	*/
			fflg = TRUE;
			continue;
		case 'g':		/*	Group ID istead if User	*/
			gflg = TRUE;
			continue;
		case 'i':		/*	List inode numbers	*/
			iflg = TRUE;
			continue;
		case 'l':		/*	Long listing		*/
			lflg = TRUE;
			continue;
		case 'r':		/*	Reverse sort		*/
			rflg = TRUE;
			continue;
		case 's':		/*	List filesize in blocks	*/
			sflg = TRUE;
			continue;
		case 't':		/*	Sort by time (def:name)	*/
			tflg = TRUE;
			continue;
		case 'u':		/*	Use time of last access	*/
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
		fprintf(stderr,"usage: %s [-acdefgilrstu] [file | dir ...]\n", PRMPT);
		return(-1);
	}

	if (fflg) {
		aflg = TRUE;
		eflg = FALSE;
		lflg = FALSE;
		sflg = FALSE;
		tflg = FALSE;
	}

	if (argc != 0) {
		totf = TRUE;
		root = makentry(pname(*argv), dname(*argv));
		
		for (i = 1; i < argc; ++i)
			instree(makentry(pname(argv[i]), dname(argv[i])), root);
		/*	The flag top_lev indicates that the names are 	*/
		/*	not read from a directory, but from standard	*/
		/*	input, thus status has to be fetched for these	*/
		/*	files and the total filename will be written	*/
		/*	since the files may have different paths.	*/

		top_lev = TRUE;

		listtree(root);

		top_lev = FALSE;

		/*	Test if directory-contents should be listed	*/
		if (!dflg)
				listdir(root);
		freetree(root);

	} else {
		totf = FALSE;
		root = makentry("", DOT);
		dirlist(root);
		freetree(root);
	}
	return(0);
}



/*	Function for comparing two records according to filename or time*/
long
compare(ptr1, ptr2)
register struct entry *ptr1, *ptr2;
{
long	result;
	/*	Test if sort by name or by time				*/
	if (tflg) {
		if (getstat(ptr1) == NULL || getstat(ptr2) == NULL)
			result = 0;

		else if (uflg)
			result = (long)(ptr2->e_stat->st_atime - ptr1->e_stat->st_atime);
		else if (cflg)
			result = (long)(ptr2->e_stat->st_ctime - ptr1->e_stat->st_ctime);
		else
			result = (long)(ptr2->e_stat->st_mtime - ptr1->e_stat->st_mtime);
	} else
		result = (long)strcmp(ptr1->e_fname, ptr2->e_fname);
	return(result);
}


/*	Function for recursive listing of one directory			*/
/*	===============================================			*/
dirlist(ptr)
register struct entry *ptr;
{
register struct direct	dir_entry;	/*	Directory entry		*/
register struct entry	*dir_tree = NULL;/*	Root of tree to sort	*/

	if (ptr == NULL)
			return(0);
	if (freopen(ptr->e_fname, "r", stdin) == NULL) {
		fprintf(stderr, NOOPEN, ptr->e_fname);
		return(1);
	}

	/*	Make a new tree and list it				*/
	while (fread((char *)&dir_entry, sizeof(dir_entry), 
		1, stdin) == 1) {

		/*	Test if the file is deleted			*/
		/*	or if it should not be listed			*/
		if (dir_entry.d_ino == 0 ||
			!aflg && (!strcmp(dir_entry.d_name, DOT) ||
			!strcmp(dir_entry.d_name, DOTDOT)))
			continue;

		/*	Make a new filename				*/

		if (dir_tree == NULL)
			dir_tree = makentry(ptr->e_fname, dir_entry.d_name);
		else
			instree(makentry(ptr->e_fname, dir_entry.d_name), dir_tree);
	}

/*	fclose(stdin); should not be used with freopen()		*/

	/*	List the directory					*/
	if (totf || eflg)
		printf("\n%s:\ntotal:%ld\n", ptr->e_fname, total(dir_tree));
	listtree(dir_tree);

	/*	Recurse to see if any directories here			*/
	if (eflg)
		listdir(dir_tree);

	/*	Free dynamically allocated space			*/
	freetree(dir_tree);
	dir_tree = NULL;
	return(0);
}

/*	Function for recursive listing of the directories		*/
/*	=================================================		*/
listdir(ptr)
register struct entry *ptr;
{
	register int	result;
	if (ptr == NULL)
			return(0);

	if (rflg)
		listdir(ptr->right);
	else
		listdir(ptr->left);
	if ((strcmp(dname(ptr->e_fname), DOT) && 
		strcmp(dname(ptr->e_fname), DOTDOT))) {

		/*	Test if the entry ptr-> is a directory		*/
		if (getstat(ptr) == NULL)
			result = 1;
		else if	(isdir(*ptr->e_stat) || fflg)
			result = dirlist(ptr);
	}

	if (rflg)
		listdir(ptr->left);
	else
		listdir(ptr->right);
	return(result);
}



/*	Function for recursive listing of the binary file-list tree	*/
/*	===========================================================	*/
listtree(ptr)
register struct entry *ptr;
{
	if (ptr == NULL)
			return(0);
	if (rflg)
		listtree(ptr->right);
	else
		listtree(ptr->left);

	if (!iflg && !sflg && !lflg && !top_lev || getstat(ptr) != NULL)
		fileinfo(top_lev ? ptr->e_fname : dname(ptr->e_fname), 
		ptr->e_stat,iflg,sflg,lflg,gflg,uflg,cflg);
	else
		fprintf(stderr, NOACC, ptr->e_fname);

	if (rflg)
		listtree(ptr->left);
	else
		listtree(ptr->right);
	return(0);
}



/*	Function returning total file-space occupied by a directory	*/
/*	========================================================	*/
off_t
total(ptr)
register struct entry *ptr;
{
off_t size;
unsigned short ftype;
	if (ptr == NULL || getstat(ptr) == NULL)
		return(0L);

		if ((ftype = ptr->e_stat->st_mode & S_IFMT) == S_IFCHR
					|| ftype == S_IFBLK)
			size = ptr->e_stat->st_rdev;
		else
			size = ptr->e_stat->st_size;
	return(total(ptr->left)+total(ptr->right) + size);
}
