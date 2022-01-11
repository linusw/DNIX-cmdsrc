/*
 * update - command
 *
 *	Copyright (C) DIAB T{by 1983
 *	Written by: Per-Arne Svensson 
 */

/*
 *	update file system every 30 seconds
 */

#include <signal.h>
#include <stdio.h>
#include <mac.h>

/*	Main Program							*/
/*	============							*/
main()
{
register int	process;	/*	Process number			*/
register int i;

	/*	Continue in a child process and let this process exit	*/
	if ((process = fork()) == -1)
		exit(1);
	else if (process != 0)
		exit(0);

	/*	Close the i/o files to the terminal			*/
	for(i = 0; i < _NFILE; i++){
		close(i);
	}

	update();

	while (TRUE)
		pause();
}

/*	Function for updating the file system - update			*/
/*	==============================================			*/
update()
{
	/*	Update file system					*/
	sync();

	/*	Set alarm interrupt routine pointer to 'update'		*/
	signal(SIGALRM, update);

	/*	Set alarm clock to 30 seconds				*/
	alarm(30);
}
