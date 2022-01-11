/*
 *	Routines for administrating the spooler.
 *
 *	Copyright (C) DIAB T{by 1984
 *	Written by: Per-Arne Svensson 
 */

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../cmd.h"
#include "../cmd_err.h"
#include "../h/spool.h"

/*	Function for making a lock-file - mk_lock()			*/
/*	===========================================			*/
mk_lock(file)
register char *file;
{
	register int lockfd;
	struct stat lockstat;

	if(( lockfd = creat(file,0)) < 0){
		return(-1);
	}
	if(fstat(lockfd,&lockstat) < 0 || lockstat.st_mode != 0100000){
		return(-1);
	}
	close(lockfd);
	return(0);
}
