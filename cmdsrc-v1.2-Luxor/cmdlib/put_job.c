/*
 *	Routines for administrating the spooler queue.
 *
 *	Copyright (C) DIAB T{by 1984
 *	Written by: Per-Arne Svensson 
 */
/*
 *	Modified by: Magnus Hedner
 *			(Nov 1984)
 */

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <ctype.h>
#include "../cmd.h"
#include "../cmd_err.h"
#include "../h/spool.h"

/*	Function for writing an entry in the queue file put_job()	*/
/*	=========================================================	*/
put_job(job)
register struct	sp_entry *job;		/*	Queue record pointer	*/
{
	FILE	*str;
	static	char	spool_pre[] = SPOOL_PRE;
	char	*chrptr = spool_pre, *dname(), *mktemp();
	strcpy(spool_pre, SPOOL_PRE);

	/*	Make a spool_pre name					*/
	{
		register char	*ptr1 = spool_pre, *ptr2 = job->jfile;
		while (*ptr1++ != 'X');
		--ptr1;
		*ptr1++ = (char)job->priority + '0';
		while (*ptr2++ != '_') ;
		while ((*ptr1++ = *ptr2++));
	}

	/*	Try to create the pre-entry file			*/
	if ((str = fopen(spool_pre,"w")) == NULL) {
		fprintf(stderr, NOOPEN, spool_pre);
		return(-1);
	}

	fprintf(str, "%d;%d;%15s;%s;%s;%s;%s;%s\n",
	job->priority,				/*	Job priority	*/
	job->header,				/*	Header count	*/
	job->timestr,				/*	Current time	*/
	job->jfile,				/*	Job file	*/
	job->banner,				/*	Banner string	*/
	job->device,				/*	Device name	*/
	job->ofile,				/*	Original file	*/
	job->mail);				/*	Mail message	*/

	fclose(str);
	return(0);
}
