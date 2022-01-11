/*	Function for making a file-list entry				*/
/*	=====================================				*/
/*									*/
/*	Copyright (C) DIAB T{by 1983					*/
/*	Written by: Per-Arne Svensson 					*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../cmd.h"
#include "../cmd_err.h"

struct entry *
makentry(path, name)
char *path;
char *name;
{
	char	*malloc();	/*	Allocate memory			*/
	char	*name_mrg();	/*	Merge path and directory names	*/
	static int	endofmem = 0;
	register struct	entry *newentry;

	if	(endofmem) return(NULL);

	newentry = (struct entry *)malloc(sizeof(struct entry));
	if	(newentry==NULL) {
	/*	End of memory		*/
		fprintf(stderr, EOFMEM);
		endofmem = TRUE;
		return(NULL);
	}
	if	(endofmem) return(NULL);

	newentry->e_fname = (char *)malloc(strlen(path)+strlen(name)+2);
	if	(newentry->e_fname==NULL) {
	/*	End of memory		*/
		fprintf(stderr, EOFMEM);
		endofmem = TRUE;
		return(NULL);
	}


	/*	Initiate file-name field				*/
	strcpy(newentry->e_fname, name_mrg(path, name));

	/*	Initiate pointers					*/
	newentry->left = newentry->right = NULL;
	newentry->e_stat = NULL;
	return(newentry);
}
