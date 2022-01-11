/*	Function for merging a file-path-name & file-dir-name name_mrg()*/
/*	================================================================*/
/*									*/
/*	Copyright (C) DIAB T{by 1983					*/
/*	Written by: Per-Arne Svensson 					*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/dir.h>
#include "../cmd.h"
#include "../cmd_err.h"

char *
name_mrg(path_name, dir_name)
char	*path_name, *dir_name;
{
static	char	fnam[MAXN];
char	*dname();
	strcpy(fnam, path_name);
	if (strlen(path_name)+DIRSIZ+2 > MAXN)
			fprintf(stderr, LNGNAM, path_name);
	else {
		if (strcmp(path_name, "") && 
			strcmp(path_name, SDELIM) && 
			strcmp(dir_name, ""))
			strcat(fnam, SDELIM);
		strcat(fnam, dname(dir_name));
	}
	return(fnam);
}

