/*	Function for inserting into a preorder-sorted tree		*/
/*	==================================================		*/
/*									*/
/*	Copyright (C) DIAB T{by 1983					*/
/*	Written by: Per-Arne Svensson 					*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../cmd.h"

instree(entryptr, nodeptr)
register struct entry *entryptr, *nodeptr;
{
extern	long compare();
	/*	Sort in the entry according to the sorting-order	*/
	if (compare(entryptr, nodeptr) > 0) {
		if (nodeptr->right == NULL) {
			nodeptr->right = entryptr;
			return(0);
		}
		else {
			instree(entryptr, nodeptr->right);
			return(0);
		}
	}
	else {
		if (nodeptr->left == NULL) {
			nodeptr->left = entryptr;
			return(0);
		}
		else {
			instree(entryptr, nodeptr->left);
			return(0);
		}
	}
}
