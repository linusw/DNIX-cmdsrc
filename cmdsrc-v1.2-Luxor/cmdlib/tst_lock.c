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

/*	Function for testing a lock-file - tst_lock()			*/
/*	=============================================			*/
tst_lock(file)
register char	*file;
{
	return(access(file, 0));
}
