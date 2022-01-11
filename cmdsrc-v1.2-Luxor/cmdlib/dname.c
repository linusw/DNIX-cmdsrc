/*									*/
/*	dname - function returning directory-name-string of parameter	*/
/*	parameter must be a file-name or directory-name			*/
/*	=============================================================	*/
/*									*/
/*	Copyright (C) DIAB T{by 1983					*/
/*	Written by: Per-Arne Svensson 					*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/dir.h>
#include "../cmd.h"	/*	Include-file for commands		*/

char *
dname(name)
register char *name;
{
	static		char	buf[DIRSIZ+1]='\0';
	register char *p;

	p = name;

	/* While not end-of-string	*/
	while	(*p)
		if (*p++ == DELIM) name = p;
	strncpy(buf, name, DIRSIZ);
	return(buf);
}
