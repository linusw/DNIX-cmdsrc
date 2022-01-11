/*	Function for making a status block entry			*/
/*	========================================			*/
/*									*/
/*	Copyright (C) DIAB T{by 1983					*/
/*	Written by: Per-Arne Svensson 					*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../cmd.h"
#include "../cmd_err.h"

struct stat *
getstat(ptr)
struct entry *ptr;
{
extern	char *malloc();
static int	endofmem = 0;

	if (ptr->e_stat != NULL)
		return(ptr->e_stat);

	if	(endofmem) return(NULL);

	ptr->e_stat = (struct stat *)malloc(sizeof(struct stat));
	if (ptr->e_stat == NULL) {
	/*	End of memory		*/
		fprintf(stderr, EOFMEM);
		endofmem = TRUE;
		return(NULL);
	}
	if	(stat(ptr->e_fname, ptr->e_stat) < 0) {
		free(ptr->e_stat);
		ptr->e_stat = NULL;
	}
	return(ptr->e_stat);
}
