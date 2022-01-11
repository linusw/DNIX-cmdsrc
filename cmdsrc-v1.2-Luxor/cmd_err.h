/*									*/
/*	cmd_err.h	 - error messages for DENIX commands		*/
/*	====================================================		*/
/*									*/

#ifdef	LIB
extern	char	*PRMPT;
#else
char	*PRMPT="error:";
#endif

/*	Query prompts:	*/

#define	LOGIN	"login: "
#define	PASSWD	"password:"
#define OLDPWD	"old password:"
#define	NEWPWD	"new password:"
#define	RTPPWD	"retype new password:"
#define COPY	"copy file %s ? [y/n]:"
#define SPCOPY	"copy special file %s ? [y/n]:"
#define	EXMDIR	"examine directory %s ? [y/n]:"
#define	RMVDIR	"remove directory %s ? [y/n]:"
#define	RMFILE	"remove file %s ? [y/n]:"
#define	OVRMSG	"override? [y/n]:"

/*	Messages	*/

#define	BRDCAST	"Broadcast Message ... \n\n"
#define	MAILMSG	"You have mail.\n"
#define	INCLOG	"Login incorrect\n"
#define	PAGEHDR	"%s Page %d\n"
#define	CHGPWD	"%s:  changing password for: %s\n", PRMPT
#define	PASSWD1	"%s:  please use at least one non-numeric character\n", PRMPT
#define	PASSWD2	"%s:  please use a longer password\n", PRMPT
#define	UCHPWD	"%s:  password unchanged\n", PRMPT
#define SUSWTCH	"%s:  superuser only may set %s flag\n", PRMPT
#define	SUONLY	"%s:  this command must have superuser privilleges\n", PRMPT
#define	ONLOWN	"%s:  \"%s\" must be the owner of this command\n", PRMPT
#define	NOPERM	"%s:  no permission\n", PRMPT
#define	BADPWD	"%s:  bad password\n", PRMPT
#define	BADSW	"%s:  bad switch %s\n", PRMPT
#define BADFORM "%s:  bad format character %s\n", PRMPT
#define BADFILE	"%s:  bad file %s, has mode %0o:\n", PRMPT
#define BADFMT	"%s:  bad format on file: %s\n", PRMPT
#define BADMODE	"%s:  invalid mode: %s \n", PRMPT
#define BADUID	"%s:  unknown user id: %s \n", PRMPT
#define BADGID	"%s:  unknown group id: %s \n", PRMPT
#define	BADGNR "%s:  bad group number %d \n", PRMPT
#define	BADCONV	"%s:  bad conversion\n", PRMPT
#define	BADSYN	"%s:  bad syntax %s\n", PRMPT
#define	MSCONJ	"%s:  missing conjunction\n", PRMPT
#define	ARGEXP	"%s:  argument expected\n", PRMPT
#define	EXPECT	"%s:  %s expected\n", PRMPT
#define	UNKNOPR	"%s:  unknown operator %s\n", PRMPT
#define	NOACC	"%s:  cannot access %s\n", PRMPT
#define	FISDIR	"%s:  %s is a directory \n", PRMPT
#define	FIDENT	"%s:  %s and %s are identical \n", PRMPT
#define	MODE	"%s:  %s: has mode:%s\n", PRMPT
#define	AGAIN	"%s:  try again\n", PRMPT
#define	NOEXEC	"%s:  cannot execute: %s\n", PRMPT
#define	ABNTERM	"%s:  abnormal termination of: %s\n", PRMPT
#define NOCHMOD	"%s:  cannot change mode on file: %s\n", PRMPT
#define NOCHOWN	"%s:  cannot change owner on file: %s\n", PRMPT
#define NOCHGRP	"%s:  cannot change group on file: %s\n", PRMPT
#define NOCHVER	"%s:  cannot change version nr on file: %s\n", PRMPT
#define	FAILMV	"%s:  failed to move %s to %s\n", PRMPT
#define	NOLINK	"%s:  cannot link %s to %s \n", PRMPT
#define	NOUNLN	"%s:  cannot unlink %s\n", PRMPT
#define	NOREMOV	"%s:  cannot remove %s\n", PRMPT
#define	NODIR	"%s:  %s is not a directory\n", PRMPT
#define	LNGNAM	"%s:  %s name too long\n", PRMPT
#define	FEXIST	"%s:  %s already exists\n", PRMPT
#define	FILIDN	"%s:  files are identical\n", PRMPT
#define	NOPRNT	"%s:  cannot locate parent: %s\n", PRMPT
#define	NRDACC	"%s:  no read access to %s\n", PRMPT
#define	NWRACC	"%s:  no write access to %s\n", PRMPT
#define	WRPROT	"%s:  %s write protected \n", PRMPT
#define	NBTDEV	"%s:  can't move directories between devices\n", PRMPT
#define	SMPATH	"%s:  %s already contains the directory %s \nwhich can't be removed\n", PRMPT
#define	NORENM	"%s:  cannot rename %s \n", PRMPT
#define	NOPDIR	"%s:  paths containing %s are not allowed \n", PRMPT
#define	NOOPEN	"%s:  cannot open file %s\n", PRMPT
#define	NOCPSF	"%s:  cannot copy a file to itself %s\n", PRMPT
#define	NOCREA	"%s:  cannot create file %s\n", PRMPT
#define NOMKDIR	"%s:  cannot make directory %s\n", PRMPT
#define NORMDIR	"%s:  cannot remove directory %s\n", PRMPT
#define	NOCHDIR	"%s:  cannot change directory %s\n", PRMPT
#define	NOTMTD	"%s:  device not mounted %s\n", PRMPT
#define TMNFLS	"%s:  too many files\n", PRMPT
#define	WRERR	"%s:  write error %s\n", PRMPT
#define	RDERR	"%s:  read error %s\n", PRMPT
#define INISOUT	"%s:  input %s is output\n", PRMPT
#define	NOOVWR	"%s:  cannot overwrite %s\n", PRMPT
#define	NOEMPTY	"%s:  %s is not empty\n", PRMPT
#define EOFMEM	"%s:  no memory available\n", PRMPT
#define	REQUEST	"REQUEST"
#define	SUCCESS	"SUCCESS"
#define	FAILED	"FAILED"
#define	REMOVED	"REMOVED"
#define	NOMAIL	"Could not send mail"
#define	SORRY	"Sorry\n"
