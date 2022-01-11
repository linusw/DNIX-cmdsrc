/*
 *	Routines for administrating the spooler.
 *
 *	Copyright (C) DIAB T{by 1984
 *	Written by: Per-Arne Svensson 
 */

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include "../cmd.h"
#include "../cmd_err.h"
#include "../h/spool.h"

/*	Function for removing a lock-file - rmv_lock()			*/
/*	=============================================			*/
rmv_lock(file)
register char	*file;
{
	if (unlink(file) != 0) {
		fprintf(stderr, NOUNLN, file);
		return(-1);
	}
	else
		return(0);
}
