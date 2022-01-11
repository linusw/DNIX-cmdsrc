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
#include <ctype.h>
#include <sys/types.h>
#include <sys/dir.h>
#include "../cmd.h"
#include "../cmd_err.h"
#include "../h/spool.h"

/*	Function for reading an entry in the queue file get_job()	*/
/*	=========================================================	*/
get_job(sp_q)
register struct sp_entry *sp_q;
{

	register int	i, chars = 0;
	register	FILE	*dir, *str;
	register	struct	direct	dir_entry;
	register	char	chr;
	int		res = 0, pri = 0, maxpri = -1;
	char		*name ,*entry, *dname(), *pname();
	static	char	spool_entry[] = SPOOL_ET;
	static	char	spool_dir[] = SPOOL_DIR;

	/*	Try to open the directory				*/
	if ((dir = fopen(spool_dir,"r")) == NULL){
		fprintf(stderr, NOOPEN, spool_dir);
		return(-1);
	}

	/*	Find string to compare with				*/
		entry = spool_entry;
		name = entry;
		while (*entry++ != 'X') chars++;

	/*	Scan the directory and search for spool_entry files	*/
	while (fread(&dir_entry,sizeof(dir_entry), 1, dir) != NULL) {
		for (i=0;i< chars;i++)
			if (dir_entry.d_name[i] != name[i])
				break;
		if (i != chars)
			continue;
		if (isdigit((chr = dir_entry.d_name[chars]))) {
			pri = (int) (chr - '0');
		} else {
			pri = 0;
		}

		if (pri > maxpri) {
			if((str = fopen(dir_entry.d_name,"r")) == NULL){
				fprintf(stderr,NOOPEN,dir_entry.d_name);
				continue;
			}

			/*	Initialize the fields			*/
			strcpy(sp_q->efile,dir_entry.d_name);
			sp_q->header = sp_q->priority = 0; 
			*sp_q->timestr = 
			*sp_q->jfile = 
			*sp_q->banner = 
			*sp_q->device = 
			*sp_q->ofile = 
			*sp_q->mail = '\0';

			fscanf(str, "%d;%d;%[^;];%[^;];%[^;];%[^;];%[^;];%[^\n]\n",
			&sp_q->priority,	/*	Job priority	*/
			&sp_q->header,		/*	Header count	*/
			sp_q->timestr,		/*	Current time	*/
			sp_q->jfile,		/*	Job file	*/
			sp_q->banner,		/*	Banner string	*/
			sp_q->device,		/*	Device name	*/
			sp_q->ofile,		/*	Original file	*/
			sp_q->mail);		/*	Mail message	*/
			maxpri = pri;
			res = -1;
			fclose(str);
		}
	}
	fclose(dir);
	return(res);
}
