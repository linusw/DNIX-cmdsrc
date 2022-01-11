/*	Include file for DNIX commands
 *
 *	Copyright (C) DIAB T{by 1983
 *	Written by: Per-Arne Svensson 
 */

#define	ROOT_NAME	"root"
#define	SHELL_FILE	"/bin/sh"
#define	WTMP_FILE	"/usr/adm/wtmp"
#define	UTMP_FILE	"/etc/utmp"
#define	PASSWD_FILE	"/etc/passwd"
#define	PASSWD_LOCK	"/tmp/passwd_lock"
#define	GROUP_FILE	"/etc/group"
#define	GROUP_LOCK	"/tmp/group_lock"
#define	MOTD_FILE	"/etc/motd"
#define	CRON_FILE	"/usr/lib/crontab"
#define TEMP_FILE	"/tmp/XXXXXX"
#define	MAIL_DIR	"/usr/spool/mail"
#define	NULL_DEV	"/dev/null"
#define	MTAB_FILE	"/etc/mtab"
#define	SYS_DIR		"..systemfiles"
#define	VOLUME_FILE	"volumedescr"
#define	BITMAP_FILE	"bitmap"
#define	KILL_CH		'@'
#define	ERASE_CH	'#'
#define TRUE 1
#define FALSE 0
#define MAXN		100	/*	Maximum file-name-length	*/
#define	MAXWIDTH	160	/*	Maximum line length on printer	*/
#define	STACKINC	512	/*	Stack increment size		*/
#define DOT		"."	/*	Current directory		*/
#define DOTDOT		".."	/*	Previous directory		*/
#define DELIM		'/'	/*	Numerical delimiter		*/
#define SDELIM		"/"	/*	String delimiter 		*/
#define MODEBITS	07777	/*	Mode bit mask			*/
#ifndef	ROOTINO
#define ROOTINO		2	/*	Root-inode			*/
#endif

/*	Status mode-bits						*/
/*	================						*/
#define	USER	05700		/*	User's bits			*/
#define	GROUP	02070		/*	Group's bits			*/
#define	OTHERS	00007		/*	Other's bits			*/
#define	ALL	01777		/*	All's bits			*/
#define	READ	00444		/*	Read permission			*/
#define	WRITE	00222		/*	Write permission		*/
#define	EXEC	00111		/*	Execute permission		*/
#define	SETID	06000		/*	Set user/group id on exec	*/
#define	STICKY	01000		/*	Sticky bit			*/

/*	Character-type macros						*/
/*	=====================						*/
#define	STRIP	0177		/*	Strip off lowest bits in char	*/

/*	Structure for a file-list entry					*/
/*	===============================					*/
struct	entry {
	struct	entry *left;		/*	Left Treepointer	*/
	struct	entry *right;		/*	Right Treepointer	*/
	char	*e_fname;		/*	Filename pointer	*/
	struct	stat	*e_stat;	/*	Status-block pointer	*/
	};

#define	MAX(a,b)	((a)>(b)?(a):(b))
#define	alloc(a)	malloc(a)
#define isdir(status)	(((status).st_mode & S_IFMT) == S_IFDIR)
#define	SCPYN(a, b)	strncpy(a, b, sizeof(a))
