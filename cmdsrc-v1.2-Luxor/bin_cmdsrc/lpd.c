/*
 *	lpd
 *
 *	Copyright (C) DIAB T{by 1984
 *	Written by: Per-Arne Svensson 
 */
/*
 *	Modified by: Magnus Hedner
 *			( Nov 1984 )
 */

/*
 *	Spooler daemon
 */

#include <ctype.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include "../cmd.h"
#include "../cmd_err.h"
#include "../h/spool.h"

/*	Standard input stream buffer					*/
/*	============================					*/

char	devbuf[BUFSIZ];		/*	Buffer for device		*/
#define BSIZE 256

char b1[BSIZE], b2[BSIZE];

/*	External functions						*/
/*	==================						*/
char	*dname();		/*	Get directory-name of file	*/
char	*IDname();		/*	Get UID/GID-string		*/
int	geteuid();		/*	Get effective UID number	*/
int	get_job();		/*	Fetch a queue file entry	*/
int	put_job();		/*	Make a queue file entry		*/
int	rmv_job();		/*	Remove a job from the queue	*/


/*	Global variables						*/
/*	================						*/

struct	sp_entry sp;		/*	Spooler queue record entry	*/
FILE	*dev_str;		/* Device file stream */
FILE	*job_str;
int	res;			/*	result from do_copy		*/
extern int errno;		/*	result from various calls	*/

/*			Functions					*/
/*			=========					*/

int	abort();		/*	Call abort and return errorcode	*/
int	do_job();		/*	Execute a queue job		*/
int	send_mail();		/*	Send mail to the user		*/

/*	Main program							*/
/*	============							*/
main(argc, argv)
char *argv[];
int  argc;

{
	/*	Initiate prompt string					*/
	PRMPT = *argv;

	++argv;
	--argc;

	umask(0);

	/*	Check argument count					*/
	if (argc > 0) {
		fprintf(stderr, "usage: %s \n", PRMPT);
		exit(1);
	}

	/*	Start spooler						*/
	setup();

	/*	Scan through the queue file and start all the device	*/
	/*	spoolers that are necessary, then sleep for a while	*/
	/*	until the queue is empty				*/

	while(get_job(&sp)){
		do_job();
	}
	abort();
}



/*	Function for setting up the daemon - setup()			*/
/*	============================================			*/
setup()
{
	struct stat statblk;		/*	File status block	*/

	setuid(SPOOL_UID);
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGALRM, abort);

	/*	Test if the lock is active				*/
	if (tst_lock(SPOOL_LOCK) != -1)
		exit(0);
	else if (mk_lock(SPOOL_LOCK) == -1)
		exit(2);

	/*	Change directory to the path of the spool jobs		*/
	if (chdir(SPOOL_DIR) != 0){
		fprintf(stderr, NOCHDIR, SPOOL_DIR);
		rmv_lock(SPOOL_LOCK);
		exit(3);
	}

	if (stat(SP_ERRFILE, &statblk) == -1)
		freopen(SP_ERRFILE, "w", stderr);
	else {
		if (statblk.st_size > SP_MAXLOG) {
			unlink(SP_OLDERR);
			link(SP_ERRFILE, SP_OLDERR);
			unlink(SP_ERRFILE);
			freopen(SP_ERRFILE, "w", stderr);
		} 
		else
			freopen(SP_ERRFILE, "a", stderr);
	}
	close(fileno(stdin));
	close(fileno(stdout));

	/*	Execute in the background				*/
		if (fork())
			exit(0);
}

/*	Function for executing a job - do_job()				*/
/*	=======================================				*/
do_job()
{

	/*	Try to open the job file			*/
	if ((job_str = fopen(sp.jfile, "r")) == NULL) {
		fprintf(stderr, NOOPEN, sp.jfile);
		fprintf(stderr, "errno: %d\n", errno);
		rmv_job(&sp);
	} 
	else {

		/* Open the device */
		if((dev_str = fopen(sp.device,"w")) == NULL){
			fprintf(stderr,NOOPEN,sp.device);
			rmv_lock(SPOOL_LOCK);
			exit(2);
		}
		setbuf(dev_str,devbuf);

		/*	Copy the job to the device			*/
		/*	==========================			*/
		putctl('\f');
		while(sp.header--){
			bigtext(dname(sp.banner), dev_str);
			bigtext(dname(sp.jfile), dev_str);
			bigtext(dname(sp.ofile), dev_str);
			putctl('\n');
			putctl('\f');
		}

		res = 0;	/*	Don't accumulate		*/
		if(do_copy()){
			fprintf(stderr, WRERR, sp.device);
		} 
		putctl('\f');
		fclose(dev_str);
		fclose(job_str);
		/*	Remove the entry corresponding to the job file	*/
		if (rmv_job(&sp) == -1)
			fprintf(stderr, NOREMOV, sp.jfile);
	}

	send_mail(&sp);

}


do_copy()
{
	register int chr;
	register int pos;
	register int pos1,pos2;

	pos1 = 0;
	pos2 = 0;
	pos = 0;
	while ((chr = getc(job_str)) != EOF) {
		if (isprint (chr)) {
			if (pos < BSIZE) {
				if (pos >= pos1 || b1[pos] == ' ') {
					while (pos >= pos1)
						b1[pos1++] = ' ';
				} else {
					if (pos < pos2 && b2[pos] != ' '){
						prbuf(b2,pos2,1);
						pos2 = 0;
					}
					while (pos >= pos2)
						b2[pos2++] = ' ';
					b2[pos] = b1[pos];
				}
				b1[pos++] = chr;
			}
		} else {
			switch (chr) {

			case ' ':
				pos++;
				break;
			
			case '\b':
				if (pos)
					pos--;
				break;

			case '\t':
				pos = (pos + 8) & -8;
				break;

			case '\r':
				pos = 0;
				break;

			case '\n':
			case '\f':
			default:
				prbuf(b2,pos2,1);
				pos2 = 0;
				prbuf(b1,pos1,0);
				pos1 = 0;
				putctl(chr);
				pos = 0;
				break;

			}
		}
	}
	prbuf(b2,pos2,1);
	prbuf(b1,pos1,0);
	return(res);
}
prbuf(buf,pos,flag)
register char *buf;
register int pos;
int flag;
{
	register char *lim;

	lim = buf + pos;
	if ( buf < lim) {
		while(buf < lim){
			if(putc(*buf, dev_str) != *buf++)
				res = 1;
		}
		if(flag){
			putctl('\r');
		}
	}
}

/*	Function for sending mail - send_mail()				*/
/*	=======================================				*/
send_mail(sp_q)
register struct	sp_entry *sp_q;		/*	Queue record pointer	*/
{
	static	char	mail_msg[16] = "mail ";
	FILE	*mail_str, *popen();
	char	*uid_str, *IDname();
	struct	stat	statblk;

	/*	Test if any mail should be sent				*/
	if (!*sp_q->mail)
		return(0);

	/*	Find the UID-name for the owner of the file		*/
	if (stat(sp_q->efile, &statblk) != -1)
		uid_str = IDname(statblk.st_uid, FALSE);
	else
		uid_str = sp_q->banner;

	/*	Copy mail_tsk and the mail into a string		*/
	strncpy(mail_msg+5, uid_str, 8);

	if ((mail_str = popen(mail_msg, "w")) == NULL) {
		return(-1);
	} 
	else {
		fprintf(mail_str, "%s: %s %s\n", 
		PRMPT, 
		sp_q->mail, 
		sp_q->jfile);
		pclose(mail_str);
	}
	return(0);
}

/*	Function for trapping signals abort				*/
/*	===================================				*/
abort()
{
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGALRM, SIG_IGN);

	/*	Remove spooler-lock					*/
	rmv_lock(SPOOL_LOCK);

	exit(0);
}

/*
 *	Put control character 
 *	Refuse to put form feed character twice
 */
putctl(c)
register int c;
{
	static int oldchr = '\f';

	if( !(c == '\f' && c == oldchr))
		if(putc(c, dev_str) != c)
			res = 1;
	oldchr = c;
}
