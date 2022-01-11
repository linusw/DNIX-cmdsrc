/*	Function for removing a directory - rmvdir			*/
/*	==========================================			*/
/*									*/
/*	Copyright (C) DIAB T{by 1983					*/
/*	Written by: Per-Arne Svensson 					*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <signal.h>
#include "../cmd.h"
#include "../cmd_err.h"

rmvdir(file_ptr)
struct	entry	*file_ptr;	/*	Directory file entry pointer	*/
{
int	(*sighup)(), (*sigint)(), (*sigquit)(), (*sigpipe)(), (*sigterm)();
int	ret____ = 0;
static	char command[MAXN+6] = "rmdir ";

	if (file_ptr == NULL)
		return(1);

	/*	Disable signals (to make rmvdir() work)			*/
	/*	=======================================			*/
	sighup	= signal(SIGHUP, SIG_IGN);
	sigint	= signal(SIGINT, SIG_IGN);
	sigquit	= signal(SIGQUIT, SIG_IGN);
	sigpipe	= signal(SIGPIPE, SIG_IGN);
	sigterm	= signal(SIGTERM, SIG_IGN);
	if (geteuid() == 0)	/*	Superuser-permission		*/
		ret____	= rmd(file_ptr);
	else {
		strcpy(command+6, file_ptr->e_fname);
		if (system(command) != 0) {
			fprintf(stderr, NOEXEC, command);
			ret____ = 1;
		}
	}

	/*	Restore signals						*/
	/*	===============						*/
	signal(SIGHUP, sighup);
	signal(SIGINT, sigint);
	signal(SIGQUIT, sigquit);
	signal(SIGPIPE, sigpipe);
	signal(SIGTERM, sigterm);
	if (file_ptr->e_stat != NULL) {
		free(file_ptr->e_stat);
		file_ptr->e_stat = NULL;
	}
	return(ret____);
}

static	rmd(entry)
struct	entry	*entry;		/*	File entry pointer		*/
{

/*	External functions						*/
/*	==================						*/
char	*pname();		/*	Get path-name of file		*/
char	*dname();		/*	Get directory-name of file	*/
char	*name_mrg();		/*	Merge path and directory name	*/
struct	entry *makentry();	/*	Make one tree entry		*/
struct	entry *freetree();	/*	Free memory occupied by tree	*/
struct	stat  *getstat();	/*	Make one status block entry	*/

/*	Local variables							*/
/*	===============							*/
char	dirname[MAXN], parname[MAXN];
int	filedsc;
struct	stat file_st, cd_st;
struct	direct	dir_entry;
struct	entry	*dot_ptr;	/*	Directory file entry pointer	*/
struct	entry	*dotdot_ptr;	/*	Parent dir file entry pointer	*/

	/*	Get file status						*/
	if (getstat(entry) == NULL) {
		fprintf(stderr, NOACC, entry->e_fname);
		goto error;
	}

	/*	Get current directory file status			*/
	if (stat("", &cd_st) != 0) {
		fprintf(stderr, NOACC, "\"\"");
		goto error;
	}

	/*	Test if file is a directory				*/
	if (!isdir(*(entry->e_stat))) {
		fprintf(stderr, NODIR, entry->e_fname);
		goto error;
	}

	/*	Test if file and current directory are equal		*/
	if ((entry->e_stat)->st_ino == cd_st.st_ino && 
		(entry->e_stat)->st_dev == cd_st.st_dev) {
		fprintf(stderr, NORMDIR, DOT);
		goto error;
	}

	/*	Test if the directory is empty				*/
	if ((filedsc = open(entry->e_fname, 0)) == -1) {
		fprintf(stderr, NOOPEN, entry->e_fname);
		goto error;
	}

	while (read(filedsc, (char *)&dir_entry, sizeof(dir_entry)) == 
		sizeof(dir_entry)) {
		if (!dir_entry.d_ino ||
			!strcmp(dir_entry.d_name, DOT) ||
			!strcmp(dir_entry.d_name, DOTDOT))
			continue;
		else {
			fprintf(stderr, NOEMPTY, entry->e_fname);
			close(filedsc);
			goto error;
		}
	}
	close(filedsc);

	/*	Test if an attempt has been made to remove . or ..	*/
	if (!strcmp(dname(entry->e_fname), DOT) || 
		!strcmp(dname(entry->e_fname), DOTDOT)) {
		fprintf(stderr, NORMDIR, dname(entry->e_fname));
		goto error;
	}

	/*	Test if the directory is write protected		*/
	if (access(entry->e_fname, 02) != 0) {
		fprintf(stderr, NWRACC, entry->e_fname);
		goto error;
	}

	/*	Try to unlink . and ..					*/

	if ((dot_ptr = makentry(entry->e_fname, DOT)) == NULL)
		goto error;

	if ((dotdot_ptr = makentry(entry->e_fname, DOTDOT)) == NULL)
		goto error;

	if (unlink(dot_ptr->e_fname) != 0) {
		fprintf(stderr, NOUNLN, dot_ptr->e_fname);
		goto error;
	}

	if (unlink(dotdot_ptr->e_fname) != 0) {
		fprintf(stderr, NOUNLN, dotdot_ptr->e_fname);
		goto error;
	}

	/*	Try to unlink the directory				*/
	if (unlink(entry->e_fname) != 0) {
		fprintf(stderr, NOUNLN, entry->e_fname);
		goto error;
	}
		freetree(dot_ptr);
		freetree(dotdot_ptr);
	return(0);
error:
		freetree(dot_ptr);
		freetree(dotdot_ptr);
	return(1);
}
