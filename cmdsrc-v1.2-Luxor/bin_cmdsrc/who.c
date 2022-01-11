/*
 *	who command
 *
 *	Copyright (C) DIAB T{by 1983
 *	Written by: Per-Arne Svensson 
 */

/*
 *	who [file_name] ||  [am I]
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <utmp.h>
#include <pwd.h>
#include "../cmd.h"
#include "../cmd_err.h"

/*	External functions						*/
/*	==================						*/
struct passwd *getpwuid();
int	setpwent();		/*	Reset and open password file	*/
int	endpwent();		/*	Close password file		*/
extern	char	*pname();	/*	Get path-name of file		*/
extern	char	*dname();	/*	Get directory-name of file	*/
char	*ttyname();		/*	Get tty-name			*/
char	*strcpy(), *strncpy();	/*	Copy a string			*/
char	*strcat(), *strncat();	/*	Concatenate strings		*/
int	strcmp();		/*	Compare strings			*/
int	strlen();		/*	Length of string		*/
int	ttyslot();		/*	Get file pos of tty record	*/

/*	Global variables						*/
/*	================						*/

struct	passwd	*pwd;			/*	Password pointer	*/
struct	utmp	log_info;		/*	Record for login info	*/


/*	Main program							*/
/*	============							*/
main(argc, argv)
char *argv[];
int  argc;

{
char	*tty_name;		/*	Terminal name			*/
int	tty_fpos;		/*	Position in the tty-file	*/
int	filedsc;		/*	File descriptor			*/
char	*name_ptr;		/*	Login/Password character pointer*/
				/*	Login/Password name		*/
char	name[sizeof(log_info.ut_name)+1];
int	chr;			/*	Login character variable	*/
struct	stat	statblk;	/*	File status block		*/

	/*	Initiate prompt string					*/
	PRMPT = *argv;

	--argc;
	++argv;

	/*	Test if 'who am I' -construct				*/
	if (argc == 2) {
		if ((name_ptr = ttyname(fileno(stdin))) == NULL) {
		/*	command not invoked from a terminal, try to get	*/
		/*	as much information as possible from the 	*/
		/*	password file					*/
bad:
		pwd = getpwuid(getuid());
		SCPYN(log_info.ut_name, pwd ? pwd->pw_name : "?");
		SCPYN(log_info.ut_line, "tty??");
		time(&log_info.ut_time);
		display(&log_info);
		exit(0);
		} else
			name_ptr = dname(name_ptr);
	}

	/*	Default is to read from	the 'utmp' file that contains	*/
	/*	info about who is logged in at what terminal. Another	*/
	/*	file is  for example the 'wtmp' file			*/
	/*	that contains info about all logins and logouts		*/
	if ((filedsc = open(argc == 1 ? *argv : UTMP_FILE, 0)) != -1) {

		if (argc == 2) {
			/*	Get relative position for the		*/
			/*	tty-record in the 'ttys' file which	*/
			/*	is the same as for the 'utmp' file	*/
			tty_fpos = ttyslot();

			if(tty_fpos < 0){
				goto bad;
			}
			/*	Position to the right entry		*/
			lseek(filedsc, (long)(tty_fpos * sizeof(struct utmp)), 0);

			/*	read record				*/
			if (read(filedsc, (char *)&log_info, 
				sizeof(struct utmp)) != 
				sizeof(struct utmp))
				goto bad;

			display(&log_info);

		} else while (read(filedsc, (char *)&log_info, 
				sizeof(struct utmp)) == 
				sizeof(struct utmp))
				display(&log_info);

		close(filedsc);
	} else
		fprintf(stderr, NOOPEN, argc == 1 ? 
			*argv : UTMP_FILE);

	exit(0);
}

/*	Function for displaying info about a terminal - display()	*/
display(ptr)
struct utmp *ptr;
{
char	*ctime();
	if (*ptr->ut_name)
		printf("%-8.8s %-8.8s %.12s\n", ptr->ut_name, ptr->ut_line, 
			ctime(&ptr->ut_time)+4);
}
