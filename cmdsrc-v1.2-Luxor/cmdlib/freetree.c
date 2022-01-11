/*	Function freeing occupied tree-space				*/
/*	====================================				*/
/*									*/
/*	Copyright (C) DIAB T{by 1983					*/
/*	Written by: Per-Arne Svensson 					*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../cmd.h"

struct entry *
freetree(ptr)
register struct entry *ptr;
{
	if (ptr == NULL)
		return(NULL);
	freetree(ptr->left);
	freetree(ptr->right);
	if (ptr->e_stat != NULL)
		free((char *)ptr->e_stat);
	if (ptr->e_fname != NULL)
		free((char *)ptr->e_fname);
	free((char *)ptr);
	return(NULL);
}
