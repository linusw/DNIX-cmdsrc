/*
 *	Routines for administrating the spooler queue.
 *
 *	Copyright (C) DIAB T{by 1984
 *	Written by: Per-Arne Svensson 
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/dir.h>
#include "../cmd.h"
#include "../cmd_err.h"
#include "../h/spool.h"

/*	Function for removing a job from the queue - rmv_job()		*/
/*	======================================================		*/
rmv_job(sp)
register struct sp_entry *sp;
{
	int	res = 0;

	if (unlink(sp->jfile) !=  0 ) {
		fprintf(stderr, NOUNLN, sp->jfile);
		res = -1;
	}

	if (unlink(sp->efile) != 0) {
		fprintf(stderr, NOUNLN, sp->efile);
		res = -1;
	}
	return (res);
}
