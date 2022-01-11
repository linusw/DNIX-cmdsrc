
/*
 * time command
 *
 *	Copyright (C) DIAB T{by 1983
 *	Written by: Per-Arne Svensson 
 */

/*
 *	print real-, system- and user-time
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/times.h>
#include <sys/timeb.h>
#include <signal.h>
#include "../cmd.h"
#include "../cmd_err.h"

/*	External functions						*/
/*	==================						*/
times();			/*	Get process times record	*/
ftime();			/*	Get real time record		*/

/*	main program							*/
/*	============							*/
main(argc, argv)
register int argc;
register char *argv[];

{
int	prt_tim();
struct	tms	ex_bef, ex_aft;
struct	timeb	bef_tim, aft_tim;
int	child, exec_st;

	/*	Initiate prompt string					*/
	PRMPT = *argv;

	++argv;
	--argc;
	/*	Test if enough number of arguments			*/
	if (argc < 1) {
		fprintf(stderr, "usage: %s command\n", PRMPT);
		exit(0);
	}

	/*	Get current time before executing the command		*/
	ftime(&bef_tim);

	/*	Create a child process					*/
	child = fork();

	/*	Test if fork succeeded					*/
	if (child == -1){
		fprintf(stderr, AGAIN);
		exit(1);
	}

	/*	If we are in the child-process, execute the command	*/
	if (child == 0) {
		execvp(*argv, argv);
		/*	exec did not succeed				*/
		fprintf(stderr, NOEXEC, *argv);
		perror(**argv);
		exit(1);
	}

	/*	Disable some signals					*/
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	/*	Get process times of current process while not child	*/
	times(&ex_bef);
	while (wait(&exec_st) != child)
		times(&ex_bef);

	/*	Get process times of current process after child	*/
	times(&ex_aft);

	/*	Get current time after execution			*/
	ftime(&aft_tim);

	/*	Test if abnormal termination of process			*/
	if ((exec_st & 255) != 0)
		fprintf(stderr, ABNTERM, *argv);

	/*	Print real time as the difference before execution
	 *	and after + user and system time for the child as the
	 *	child user time before termination of child and after
	 */

	fprintf(stderr, "\n");
	prt_tim("real", (long)((aft_tim.time - bef_tim.time) * DHZ + 
		(((long)aft_tim.millitm - (long)bef_tim.millitm) * DHZ) / 1000L));
	prt_tim("user", (long)(ex_aft.tms_cutime - ex_bef.tms_cutime)); 
	prt_tim("sys ", (long)(ex_aft.tms_cstime - ex_bef.tms_cstime));
	exit(exec_st >> 8);
}

/*	Function for formatting and printingthe time value  - prt_tim	*/
/*	=============================================================	*/
prt_tim(string, tim_cnt)
char	*string;
long	tim_cnt;
{
long	tseconds, seconds, minutes, hours;
	tim_cnt /= DHZ/10L;
	tseconds = tim_cnt % 10L;
	tim_cnt	/= 10L;
	seconds	= tim_cnt % 60L;
	tim_cnt	/= 60L;
	minutes	= tim_cnt % 60L;
	tim_cnt	/= 60L;
	hours	= tim_cnt;

	fprintf(stderr, "%s", string);
	if (hours != 0)
		fprintf(stderr, "%3ld%s", hours, ":");
	else
		fprintf(stderr, "    ");
	if (minutes != 0)
		fprintf(stderr, "%2ld%s", minutes, ":");
	else
		fprintf(stderr, "   ");
	fprintf(stderr, "%2ld%s%1ld\n", seconds, ".", tseconds);
	return;
}
