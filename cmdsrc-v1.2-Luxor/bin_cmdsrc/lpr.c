/*
 *	lpr [-r] [-c] [-m'mail'] [-n] [-hn] [-d'device'] [-p'priority'] file ...
 *
 *	Copyright (C) DIAB T{by 1984
 *	Written by: Per-Arne Svensson 
 */
/*
 *	Modified by: Magnus Hedner
 *			(Nov 1984)
 */

/*
 *	Spool one or more files
 */

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <ctype.h>
#include <sys/stat.h>
#include "../cmd.h"
#include "../cmd_err.h"
#include "../h/spool.h"
#define	HMAX	10		/*	Max no of headers		*/


/*	Standard input stream buffer					*/
/*	============================					*/

char	lprbuf[BUFSIZ];		/*	Buffer for stdin		*/

/*	External functions						*/
/*	==================						*/
struct	entry *makentry();	/*	Make one tree entry		*/
struct	entry *freetree();	/*	Free memory occupied by tree	*/
char	*pname();		/*	Get path-name of file		*/
char	*dname();		/*	Get directory-name of file	*/
time_t	time();			/*	Get current time		*/
char	*mktemp();		/*	Make temporary file name	*/
char	*ctime();		/*	Convert time to ascii-string	*/
int	getuid();		/*	Get UID number			*/
int	geteuid();		/*	Get ewffective UID number	*/
char	*IDname();		/*	Get UID/GID-string		*/
int	setbuf();		/*	Assign stream I/O buffer	*/
char	*mktemp();		/*	Make temporary file name	*/
int	get_job();		/*	Fetch a queue file entry	*/
int	put_job();		/*	Make a queue file entry		*/
int	rmv_job();		/*	Remove a job from the queue	*/

/*	Global variables						*/
/*	================						*/

long	now;			/*	Current time			*/
int	sp_job;			/*	Result from making job file	*/
int	rflg	=	FALSE;	/*	Remove the file after spooling	*/
int	cflg	=	FALSE;	/*	Copy the file instead of linking*/
int	mflg	=	FALSE;	/*	Mail message when printing done	*/
struct	sp_entry sp;		/*	Spooler queue record entry	*/

/*			Functions					*/
/*			=========					*/

int	abort();		/*	Call abort and return errorcode	*/

/*	Main program							*/
/*	============							*/
main(argc, argv)
char *argv[];
int  argc;

{
register int result = 0;	/*	Return variable			*/

	/*	Initiate prompt string					*/
	PRMPT = *argv;

	++argv;
	--argc;

	/*	Initiate spooler record fields				*/
	strcpy(sp.device, LP_DEV);
	sp.priority = 5;
	sp.header = 1;

	while (*argv && **argv == '-') {
		while (*argv && *++*argv) switch (**argv) {
		case 'r':/*	Remove the file after spooling	*/
			rflg = TRUE;
			break;
		case 'c':/*	Copy the file instead of linking*/
			cflg = TRUE;
			break;
		case 'm':/*	Mail message when spooling done	*/
			mflg = TRUE;
			strncpy(sp.mail, ++*argv, MAXWIDTH);
			*argv += strlen(*argv)-1;
			break;
		case 'n':/*	Do not report by mail, default	*/
			mflg = FALSE;
			break;
		case 'h':/*	repeat header n times		*/
			sp.header = atoi(++*argv);
			if(sp.header < 0)
				sp.header = 0;
			if(sp.header > HMAX && geteuid() != 0){
				fprintf(stderr, NOPERM);
				argc = -1;
				sp.header = HMAX;
			}
			*argv += strlen(*argv)-1;
			break;
		case 'd':/*	Device for spooling is defined	*/
			strncpy(sp.device, ++*argv, MAXN);
			*argv += strlen(*argv)-1;
			break;
		case 'b':/*	Banner page from spooler	*/
			strncpy(sp.banner, ++*argv, MAXWIDTH);
			*argv += strlen(*argv)-1;
			break;
		case 'p':/*	Change spool-job priority	*/
			sp.priority = atoi(++*argv);
			if (sp.priority > 7 && geteuid() != 0) {
				fprintf(stderr, NOPERM);
				argc = -1;
			}
			if (sp.priority <1 || sp.priority >9)
				sp.priority = 5;
			*argv += strlen(*argv)-1;
			break;
		default:
			fprintf(stderr, BADSW, *argv);
			argc = -1;
		}
		argc--;
		argv++;
	}
		/*	Check argument count				*/
	if (argc < 0) {
		fprintf(stderr, "usage: %s [-r] [-c] [-m'mail'] [-n] [-hn] [-d'device'] [-p'priority'] file ...\n", PRMPT);
		exit(1);
	}


	/*	Get UID-string if banner is not defined			*/
	if (!*sp.banner) strcpy(sp.banner,IDname(getuid(), FALSE));

	if (mflg) {
		if (!*sp.mail) strcpy(sp.mail,sp.banner);
	}

	if (!*argv) {

		/*	Create a temporary file for the listing		*/
		strcpy(sp.jfile, SPOOL_JOB);
		if (access(mktemp((char *) sp.jfile), 0) != -1) {
			fprintf(stderr, FEXIST, sp.jfile);
			exit(1);
		}

		/*	Get current time and convert it to ascii	*/
		time(&now);
		strncpy(sp.timestr, ctime(&now)+4, 15);

		strcpy(sp.ofile, "");	/*	Original filename	*/

		if (put_job(&sp) != 0)
			exit(1);

		if (freopen(sp.jfile, "w", stdout) == NULL) {
			fprintf(stderr, NOCREA, sp.jfile);
			exit(1);
		}

		/*	Copy from stdin to stdout			*/

		/*	Assign stream-I/O buffer to input stream	*/
		setbuf(stdin, lprbuf);

		{
			register char chr;
			while ((chr = getchar()) != EOF)
				if ((chr = putchar(chr)) == EOF) {
					fprintf(stderr, WRERR, sp.jfile);
					if (unlink(sp.jfile) != 0)
						fprintf(stderr, NOUNLN, sp.jfile);
					exit(1);
				}
		}

		enterjob();

		/*	Close standard output stream			*/
		fclose(stdout);

		result = 0;

	} else while (*argv) {
		struct	entry	*src_entry;/*	Source file entry	*/
		struct	entry	*dst_entry;/*	Destination file entry	*/

		/*	Create a temporary file for the listing		*/
		strcpy(sp.jfile, SPOOL_JOB);
		if (access(mktemp((char *) sp.jfile), 0) != -1) {
			fprintf(stderr, FEXIST, sp.jfile);
			exit(1);
		}
		if (access(*argv, 4) == -1) {
			fprintf(stderr, NRDACC, *argv);
			++argv;
			--argc;
			continue;
		}

		/*	Get current time and convert it to ascii	*/
		time(&now);
		strncpy(sp.timestr,ctime(&now)+4, 15);

		strncpy(sp.ofile, *argv, MAXN);/*	Original filename*/

		if (put_job(&sp) != 0)
			continue;

		sp_job = rgcopy((src_entry = makentry(*argv, "")), 
			(dst_entry = makentry(sp.jfile, "")), TRUE, !cflg);

		/*	Change owner of the job file			*/
		getstat(src_entry);
		chown(dst_entry->e_fname, src_entry->e_stat->st_uid, 
					src_entry->e_stat->st_gid);

		enterjob();

		result += sp_job;
		freetree(src_entry);
		freetree(dst_entry);
		if (rflg && sp_job == 0 && unlink(*argv) != 0)
			fprintf(stderr, NOUNLN, *argv);

		++argv;
		--argc;
	}

	/*	Execute the spooler daemon				*/
	execl(SP_DAEMON, dname(SP_DAEMON), 0);

	/*	Could not execute the spooler daemon			*/

	fprintf(stderr, NOEXEC, SP_DAEMON);
	perror(PRMPT);
/*	exit(1); */
	return(result);
}

char *strchr();

enterjob()
{
	static char spool_entry[] = SPOOL_ENTRY, spool_pre[] = SPOOL_PRE;
	register char *ptr1, *ptr2, *ptr3 = sp.jfile;

	strcpy(spool_entry, SPOOL_ENTRY);
	strcpy(spool_pre, SPOOL_PRE);

	ptr1 = strchr(spool_entry, 'X');
	ptr2 = strchr(spool_pre, 'X');

	while(*ptr3++ != '_') ;

	*ptr1++ = *ptr2++ = (char)sp.priority + '0';

	while((*ptr1++ = *ptr2++ = *ptr3++));
	if(link(spool_pre, spool_entry)){
		fprintf(stderr, NOLINK, spool_entry, spool_pre);
		if(unlink(sp.jfile) != 0)
			fprintf(stderr, NOUNLN, sp.jfile);
		exit(1);
	}
	if(unlink(spool_pre) != 0){
		fprintf(stderr, NOUNLN, spool_pre);
		if(unlink(sp.jfile) != 0)
			fprintf(stderr, NOUNLN, sp.jfile);
		exit(1);
	}
}


/*	Function for trapping signals abort				*/
/*	===================================				*/
abort()
{
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	exit(1);
}

