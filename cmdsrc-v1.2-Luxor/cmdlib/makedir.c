/*	Function for making a directory - makedir			*/
/*	=========================================			*/
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

makedir(file_ptr)
struct	entry	*file_ptr;	/*	Directory file entry pointer	*/
{
int	(*sighup)(), (*sigint)(), (*sigquit)(), (*sigpipe)(), (*sigterm)();
int	ret____ = 0;
static	char command[MAXN+6] = "mkdir ";

	if (file_ptr == NULL)
		return(1);

	/*	Disable signals (to make makedir() work)		*/
	/*	========================================		*/
	sighup	= signal(SIGHUP, SIG_IGN);
	sigint	= signal(SIGINT, SIG_IGN);
	sigquit	= signal(SIGQUIT, SIG_IGN);
	sigpipe	= signal(SIGPIPE, SIG_IGN);
	sigterm	= signal(SIGTERM, SIG_IGN);
	if (geteuid() == 0)	/*	Superuser-permission		*/
		ret____	= md(file_ptr);
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

static	md(entry)
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

struct	entry	*dot_ptr;	/*	Directory file entry pointer	*/
struct	entry	*dotdot_ptr;	/*	Directory file entry pointer	*/
struct	entry	*par_ptr;	/*	Parent dir file entry pointer	*/

	if (access(entry->e_fname, 0) == 0) {
		fprintf(stderr, FEXIST, entry->e_fname);
		return(1);
	}
	if ((dot_ptr = makentry(entry->e_fname, DOT)) == NULL)
		goto error;

	if ((dotdot_ptr = makentry(entry->e_fname, DOTDOT)) == NULL)
		goto error;

	if ((par_ptr = makentry(pname(entry->e_fname), DOT)) == NULL)
		goto error;

	/*	Try to access parent-directory				*/
	if (access(par_ptr->e_fname, 2) == -1) {
		fprintf(stderr, NOPRNT, par_ptr->e_fname);
		goto error;
	}

	/*	Try to make the directory				*/
	if (mknod(entry->e_fname, 040777, 0) == -1) {
		fprintf(stderr, NOMKDIR, entry->e_fname);
		goto error;
	}

	/*	Change owner of the directory				*/
	chown(entry->e_fname, getuid(), getgid());

	/*	Try to link this directory to current directory		*/

	if (link(entry->e_fname, dot_ptr->e_fname) == -1) {
		fprintf(stderr, NOLINK, entry->e_fname, dot_ptr->e_fname);
		unlink(entry->e_fname);
		goto error;
	}

	/*	Try to link parent to parent directory			*/

	if (link(par_ptr->e_fname, dotdot_ptr->e_fname) == -1) {
		fprintf(stderr, NOLINK, par_ptr->e_fname, 
				dotdot_ptr->e_fname);
		unlink(dot_ptr->e_fname);
		unlink(entry->e_fname);
		goto error;
	}
		freetree(dot_ptr);
		freetree(dotdot_ptr);
		freetree(par_ptr);
	return(0);
error:
		freetree(dot_ptr);
		freetree(dotdot_ptr);
		freetree(par_ptr);
	return(1);
}
