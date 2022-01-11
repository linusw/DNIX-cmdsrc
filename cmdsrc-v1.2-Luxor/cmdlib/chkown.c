/*	Function verifying that a command has ceartain owner - chkown()	*/
/*	===============================================================	*/
/*									*/
/*	Copyright (C) DIAB T{by 1984					*/
/*	Written by: Per-Arne Svensson 					*/

#include <stdio.h>
#include <pwd.h>
#include "../cmd.h"
#include "../cmd_err.h"

chkown(owner)
char	*owner;
{struct passwd *getpwnam(), *pw = getpwnam(owner);
	int pwuid = pw ? pw->pw_uid : 0;
	setuid(pwuid);
	if (pwuid != geteuid()) {
		fprintf(stderr, ONLOWN, owner);
		return(-1);
	}
	return(0);
}

