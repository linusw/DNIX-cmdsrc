/*	Include file for DNIX commands
 *
 *	Copyright (C) DIAB T{by 1984
 *	Written by: Per-Arne Svensson 
 */
/*
 *	Modified by: Magnus Hedner
 *			(Nov 1984)
 */

/*
 *	In SPOOL_JOB, the underscore ("_") MUST precede
 *	the "XXXXXX" string.
 */
#ifndef	DIRSIZ
#include <sys/dir.h>
#endif
#define	LP_DEV		"/dev/lp"
#define	SPOOL_DIR	"/usr/spool/lpd"
#define	SPOOL_ENTRY	"/usr/spool/lpd/ENT_XXXXXXX"
#define	SPOOL_PRE	"/usr/spool/lpd/PRE_XXXXXXX"
#define	SPOOL_ET	"ENT_XXXXXXX"
#define	SPOOL_UID	1
#define	QUEUE_MODE	0666
#define	SPOOL_JOB	"/usr/spool/lpd/JOB_XXXXXX"
#define	SPOOL_LOCK	"/usr/spool/lpd/lock"
#define	SP_DAEMON	"/usr/lib/lpd"
#define	SP_LOGFILE	"/usr/spool/lpd/LOGFILE"
#define	SP_OLDLOG	"/usr/spool/lpd/OLDLOGFILE"
#define	SP_ERRFILE	"/usr/spool/lpd/ERRFILE"
#define	SP_OLDERR	"/usr/spool/lpd/OLDERRFILE"
#define	SP_MAXLOG	1920

/*	Structure for a spooler-queue entry				*/
/*	===================================				*/
struct	sp_entry {
	char	jfile[sizeof(SPOOL_JOB)+1];/*	Job filename		*/
	char	efile[DIRSIZ + 1];
	char	ofile[MAXN+1];		/*	Original filename	*/
	int	priority;		/*	Job priority		*/
	char	device[MAXN+1];		/*	Spool device		*/
	char	banner[MAXWIDTH+1];	/*	Banner string		*/
	char	mail[MAXWIDTH+1];	/*	Mail message		*/
	char	timestr[16];		/*	Current time		*/
	int	header;			/*	No of banner pages	*/
};

