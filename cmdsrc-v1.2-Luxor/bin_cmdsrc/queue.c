/*
 * 	queue [ - job ... ]
 *
 *	Copyright (C) DIAB T{by 1984
 *	Written by: Per-Arne Svensson 
 */

/*
 *	look at spooler queue or remove spooled jobs
 */

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../cmd.h"
#include "../cmd_err.h"
#include "../h/spool.h"

/*	External functions						*/
/*	==================						*/
int	geteuid();		/*	Get effective UID number	*/
char	*IDname();		/*	Get UID/GID-string		*/
int	setbuf();		/*	Assign stream I/O buffer	*/
char	*mktemp();		/*	Make temporary file name	*/
int	get_job();		/*	Fetch a queue file entry	*/
int	put_job();		/*	Make a queue file entry		*/
int	rmv_job();		/*	Remove a job from the queue	*/

/*	Global variables						*/
/*	================						*/

/*	Local functions							*/
/*	===============							*/
int	abort();		/*	Timeout signal routine		*/

/*	main program							*/
/*	============							*/
main(argc, argv)
register int argc;
register char *argv[];
{
	int	sp_job;		/*	Result from get_job		*/
	struct	sp_entry sp_q;	/*	Spooler queue record entry	*/

	/*	Initiate prompt string					*/
	PRMPT = *argv;

	--argc;
	++argv;

	umask(0);

	/*	Catch timeout interrupt					*/
	signal(SIGALRM, abort);

	/*	Change directory to the path of the spool jobs		*/
	if (chdir(pname(SPOOL_JOB)) != 0) {
		fprintf(stderr, NOCHDIR, pname(SPOOL_JOB));
		exit(1);
	}

	/*	Check if any jobs should be removed			*/
	if (*argv)
		if (**argv == '-') {

			if (!*++*argv) {
				++argv;
				--argc;
			}
			while (*argv) {
				/*	Remove the queue entry 		*/
				if (rmv_job(*argv) == -1)
					fprintf(stderr, NOUNLN, *argv);
				++argv;
				--argc;
			}
		} else {
			fprintf(stderr, "usage: %s [ - job ... ]\n", PRMPT);
			exit(4);
		}




	while ((sp_job = get_job(&sp_q)) != EOF)
		fprintf(stderr, "%d;%s;%s;%s;%s;%s;%s\n",
			sp_q.priority, 
			sp_q.timestr, 
			sp_q.jfile, 
			sp_q.banner, 
			sp_q.device, 
			sp_q.ofile, 
			sp_q.mail);

	exit(0);
}

/*	Routine for catching signal SIGALRM - abort()			*/
abort()
{
	fprintf(stderr, AGAIN);
	exit(1);
}
