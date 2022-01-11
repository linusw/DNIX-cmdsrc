/* defs.h */

/*
 * This file contains various definitions used by the mkusr command
 * source files.
 */

/* 1984-02-17, Peter Andersson, Luxor Datorer AB */

#define		BSIZE		256	/* input buffer size             */
#define		CSIZE		1024	/* concatenate buffer size       */
#define		BUFSIZE		512	/* file buffer size              */

#define		UPPERCASE	1	/* uppercase mark                */
#define		LOWERCASE	(-1)	/* lowercase mark                */
#define		NOTALPHA	0	/* not alphabetic mark           */

#define		TRIES		3	/* no. of retries for password   */
#define		MIXEDLEN	4	/* min mixed password length     */
#define		SAMELEN		6	/* min non-mixed password length */

#define		DIR_PROT	0755	/* protection for directories    */
#define		FIL_PROT	0644	/* protection for files          */
#define		XFIL_PROT	0755	/* protection for script files   */

#define		PWDFIL		"/etc/passwd"
#define		TMPPWDFIL	"/etc/passwd.tmp"
#define		GRPFIL		"/etc/group"
#define		TMPGRPFIL	"/etc/group.tmp"

#define		DEFAULTTERM	"vt100"		/* default terminal type */

#define		PROFILE		".profile"
#define		PROFTXT1	"stty -tabs cr0 ff0 nl0 erase '^H' kill '^X'\nTERM="
#define		PROFTXT2	"\nexport TERM\n"
