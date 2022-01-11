/*	Funktion asking a question - ask				*/
/*	================================				*/
/*									*/
/*	Copyright (C) DIAB T{by 1983					*/
/*	Written by: Per-Arne Svensson 					*/

#include <stdio.h>
#include <sys/types.h>
#include "../cmd.h"
#include "../cmd_err.h"

ask(flag, message, name)
register int flag;
char	*message, *name;
{
register int chr, chr1;
	if (flag) {
		fprintf(stderr, message, name);
		while((chr1 = getchar()) != '\n' && chr1 != EOF)
			chr = chr1;
		switch(chr) {
			case 'y':
				return(TRUE);
			default:
				return(FALSE);		
		}
	}
	else
	return(TRUE);
}
