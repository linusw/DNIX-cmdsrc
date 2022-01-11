/*
 *	copy - general copy command
 *
 *	Copyright (C) DIAB T{by 1983
 *	Written by: Per-Arne Svensson 
 */

/*
 *	copy files
 */

/*
 *	copy [-n] [-l] [-a[d]] [-m] [-r] [-t] [-v] src ... [dst]
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include "../cmd.h"
#include "../cmd_err.h"

/*	External functions						*/
/*	==================						*/
char	*pname();		/*	Get path-name of file		*/
char	*dname();		/*	Get directory-name of file	*/
int	makedir();		/*	Make directory			*/
char	*strcpy();		/*	Copy a string			*/
char	*strcat();		/*	Concatenate strings		*/
int	strcmp();		/*	Compare strings			*/
int	rgcopy(), spcopy();	/*	Copy file			*/
struct	entry *makentry();	/*	Make one tree entry		*/
struct	entry *freetree();	/*	Free memory occupied by tree	*/
struct	stat  *getstat();	/*	Make one status block entry	*/
instree();			/*	Insert entry in tree structure	*/
int	fileinfo();		/*	Detailed info about file	*/

/*	Global variables						*/
/*	================						*/

int	aflg	=	FALSE;	/*	Ask the user before copy file	*/
int	adflg	=	FALSE;	/*	Ask for every direcory if -r	*/
int	oflg	=	FALSE;	/*	Change owner to same as source	*/
int	lflg	=	FALSE;	/*	Use links whenever possible	*/
int	mflg	=	FALSE;	/*	Keep old mod- and acc- time	*/
int	nflg	=	FALSE;	/*	Requires all copied files new	*/
int	rflg	=	FALSE;	/*	Recursive copy of directories	*/
int	suflg	=	FALSE;	/*	Super-user flag			*/
int	tflg	=	FALSE;	/*	Keep tree structure unchanged	*/
int	uflg	=	FALSE;	/*	Update option (Younger)		*/
int	vflg	=	FALSE;	/*	Verbouse option			*/

/*	Local functions							*/
/*	===============							*/

dircopy();			/*	List one file (recursive)	*/

/*	Main program							*/
/*	============							*/
main(argc, argv)
char *argv[];
int  argc;
{
ino_t	r_ino_nr;		/*	Root inode number		*/
register int i;			/*	Loop variable			*/
register int result = 0;	/*	Return variable			*/
struct	entry	*dst_entry;	/*	Destination file entry		*/
struct	entry	*src_entry;	/*	Source file entry		*/

	/*	Initiate prompt string					*/
	PRMPT = *argv;

	/*	Set file creation mode mask				*/
	umask(0);

	/*	Set super-user flag					*/
	suflg = !getuid();

	++argv;
	--argc;

	while (*argv && **argv == '-') {
		while(*++*argv) switch (**argv) {
		case 'a':	/*	Ask the user before copy file	*/
			switch (*++*argv) {
			case 'd':
				adflg = TRUE;
				rflg  = TRUE;
				break;
			default:
				adflg = TRUE;
				aflg  = TRUE;
				--*argv;
				break;
			}
			continue;
		case 'o':	/*	Change owner to same as source	*/
			switch (suflg) {
			case FALSE:
				/*	Only superuser allowed		*/
				fprintf(stderr, SUSWTCH, *argv);
				break;
			default:
				oflg = TRUE;
				break;
			}
			continue;
		case 'l':	/*	Use links whenever possible	*/
			lflg = TRUE;
			continue;
		case 'm':	/*	Keep old mod- and acc- time	*/
		case 'O':
			mflg = TRUE;
			continue;
		case 'n':	/*	Requires all copied files new	*/
			nflg = TRUE;
			continue;
		case 'r':	/*	Recursive copy of directories	*/
			rflg = TRUE;
			continue;
		case 't':	/*	Keep tree structure unchanged	*/
			tflg = TRUE;
			continue;
		case 'u':	/*	Files where src younger than dst*/
			uflg = TRUE;
			continue;
		case 'v':	/*	Detailed info about every file	*/
			vflg = TRUE;
			continue;
		default:
			fprintf(stderr, BADSW, *argv);
			argc = 0;
		}
		++argv;
		--argc;
	}
	if (argc <= 0) {
		fprintf(stderr,"usage: %s [-n] [-l] [-a[d]] [-m] [-r] [-t] [-v] src ... [dst]\n", PRMPT);
		return(-1);
	}
	if (argc == 1) {
		dst_entry = makentry(DOT, "");
		argc++;
	}
	else
		dst_entry = makentry(argv[argc-1],"");

	/*	If the number of files to be copied is > 1 		*/
	/*	try to make the destination file a directory		*/
	if (argc > 2) {
		/*	If the destination file doesn't exist		*/
		if (getstat(dst_entry) == NULL) {
			if(makedir(dst_entry) != 0)
				return(-1);
		} else if (getstat(dst_entry) == NULL || 
			!isdir(*(dst_entry->e_stat))) {
			fprintf(stderr, NODIR, dst_entry->e_fname);
			return(-1);
		}
	}
	if (getstat(dst_entry) != NULL)
		r_ino_nr = dst_entry->e_stat->st_ino;

	for (i = 0; i < argc-1; ++i) {
		if ((src_entry = makentry(argv[i], "")) == NULL)
			return(1);
		result = result + dircopy(src_entry, dst_entry, r_ino_nr);
		freetree(src_entry);
	}
	return(result);
}

/*	Function for recursive copy of directory contents		*/
/*	=================================================		*/
dircopy(sourceptr, destptr, rootnode)
struct	entry	*sourceptr;	/*	Source file entry pointer	*/
struct	entry	*destptr;	/*	Destination file entry pointer	*/
ino_t	rootnode;		/*	Root inode number		*/
{
struct	entry	*tempptr;	/*	Temporary file entry pointer	*/

	/*	Test if source file is accessible			*/
	if (getstat(sourceptr) == NULL) {
		/*	Cannot access source file			*/
		fprintf(stderr, NOACC, sourceptr->e_fname);
		return(1);
	} else if (isdir(*(sourceptr->e_stat))) {
		if(getstat(destptr) == NULL) {/*	Get root status	*/
			if(makedir(destptr) != 0)
				return(1);
			else
			/*	Set same mode as source directory	*/
			chmod(destptr->e_fname, 
				sourceptr->e_stat->st_mode & ~S_IFMT);
			getstat(destptr);
		}
		if (tflg) {
			/*	Merge source dir name and dest name	*/
			tempptr = destptr;
			if ((destptr = makentry(tempptr->e_fname, 
				dname(sourceptr->e_fname))) == NULL)
				return(1);
		}
	} else if (getstat(destptr) != NULL && isdir(*(destptr->e_stat))) {
		/*	Merge source dir name and dest name		*/
		tempptr = destptr;
		if ((destptr = makentry(tempptr->e_fname, 
			dname(sourceptr->e_fname))) == NULL)
			return(1);
	}

	return(d_copy(sourceptr, destptr, rootnode));
}
static
d_copy(src_ptr, dst_ptr, root_ino)
struct	entry	*src_ptr;	/*	Source file entry pointer	*/
struct	entry	*dst_ptr;	/*	Destination file entry pointer	*/
ino_t	root_ino;
{
register FILE	*dirstrm;		/*	Directory stream I/O	*/
register struct entry	*src_tree = NULL;/*	Root of tree to copy	*/
register struct entry	*dst_tree = NULL;/*	Root of tree to copy	*/
struct direct	dir_entry;		/*	Directory entry		*/
register int result = 0;		/*	Return variable		*/

	/*	Get source file status					*/
	if (getstat(src_ptr) == NULL) {
		/*	No access to source file			*/
		fprintf(stderr, NOACC, src_ptr->e_fname);
		return(1);
	}

	getstat(dst_ptr);

	switch (src_ptr->e_stat->st_mode & S_IFMT) {
	/*	Test if the source file is a directory			*/
	case S_IFDIR:

		
		if (ask(adflg, EXMDIR, src_ptr->e_fname) == FALSE)
			return(0);
			
		/*	Create the destination directory, if it doesn't exist*/
		if (getstat(dst_ptr) == NULL) {
			if((result = makedir(dst_ptr) ) != 0)
				return(1);
			else
			/*	Set same mode as source directory	*/
			chmod(dst_ptr->e_fname, 
				src_ptr->e_stat->st_mode & ~S_IFMT);
		}
	
		/*	OK dstpath is created, or exists already, and	*/
		/*	is write enabled				*/

		/*	Open the source directory			*/
		if ((dirstrm = fopen(src_ptr->e_fname, "r")) == NULL) {
			fprintf(stderr, NOOPEN, src_ptr->e_fname);
			return(1);
		}

		/*	Make a source and a destination tree		*/
		while (fread((char *)&dir_entry, sizeof(dir_entry), 
			1, dirstrm) == 1) {

			/*	Test if the file is deleted		*/
			/*	or if it should not be copied		*/
			if (dir_entry.d_ino == 0 ||
				dir_entry.d_ino == root_ino ||
				!strcmp(dir_entry.d_name, DOT) ||
				!strcmp(dir_entry.d_name, DOTDOT))
				continue;

			/*	Make new filenames			*/

			if ((src_tree = makentry(src_ptr->e_fname, 
				dir_entry.d_name)) == NULL)
				return(1);
			if ((dst_tree = makentry(dst_ptr->e_fname, 
				dir_entry.d_name)) == NULL)
				return(1);
			
			if (getstat(src_tree) == NULL)
				/*	No access to source file	*/
				fprintf(stderr, NOACC, src_ptr->e_fname);

			else if (!isdir(*(src_tree->e_stat)) || rflg)
				d_copy(src_tree, dst_tree, root_ino);
			/*	Free dynamically allocated space	*/
			freetree(src_tree);
			freetree(dst_tree);
		}

		fclose(dirstrm);

		break;

	case S_IFREG:		/*	Regular file			*/

		/*	Ask if file should be copied			*/
		if(ask(aflg, COPY, src_ptr->e_fname) == FALSE)
			return(0);

		/*	Test if update option				*/
		if (uflg && getstat(dst_ptr) != NULL &&
			src_ptr->e_stat->st_mtime <= dst_ptr->e_stat->st_mtime)
			return(0);

		if ((result = rgcopy(src_ptr, dst_ptr, nflg, lflg)) != 0)
			return(1);
		break;
	case S_IFCHR:		/*	Character special file		*/
	case S_IFBLK:		/*	Block special file		*/
	case S_IFMPC:		/*	Multiplexed char special file	*/
	case S_IFMPB:		/*	Multiplexed block special file	*/

		if(ask(aflg, SPCOPY, src_ptr->e_fname) == FALSE)
			return(0);
		if ((result = spcopy(src_ptr, dst_ptr)) != 0)
			return(1);
		break;
	default:
		fprintf(stderr, BADFILE, src_ptr->e_fname, 
			src_ptr->e_stat->st_mode);
		return(1);
	}

	/*	Test if owner should be the same as for the source	*/
	if (oflg)
		chown(dst_ptr->e_fname, src_ptr->e_stat->st_uid, 
					src_ptr->e_stat->st_gid);

	/*	Test if old file access-tme should be kept		*/
	if (mflg)
		utime(dst_ptr->e_fname, &(src_ptr->e_stat->st_atime));

	/*	Test if verbous flag					*/
	if (vflg) {
		if (dst_ptr->e_stat != NULL) {
			free(dst_ptr->e_stat);
			dst_ptr->e_stat = NULL;
		}
		if (getstat(dst_ptr) != NULL)
			fileinfo(dst_ptr->e_fname, dst_ptr->e_stat, 
				FALSE, FALSE, TRUE, FALSE, FALSE, FALSE);
		else {
			/*	No access to destination file		*/
			fprintf(stderr, NOACC, dst_ptr->e_fname);
			result++;
		}
	}
	return(result);
}
