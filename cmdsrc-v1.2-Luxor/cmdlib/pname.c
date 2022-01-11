/*									*/
/*	pname - function returning path-name-string of parameter	*/
/*	parameter must be a file-name or directory-name			*/
/*	========================================================	*/
/*									*/
/*	Copyright (C) DIAB T{by 1983					*/
/*	Written by: Per-Arne Svensson 					*/

#include "../cmd.h"	/*	Include-file for commands	*/

char *pname(name)
register char *name;
{
	register	char	c;
	register	char	*p, *q;
	static		char	buf[MAXN];

	p = q = buf;

	while	(c = *p++ = *name++)
	    if (c == DELIM) q = p-1;

	/* Test if the path is from the root	*/
	if (q == buf && *q == DELIM) q++;

	/* End the string	*/
	*q = 0;

	return buf;
}
