/*
 *	wall [file]
 *
 *	Copyright (C) DIAB T{by 1983
 *	Written by: Per-Arne Svensson 
 */

/*
 *	Broadcast message to all users
 */

#define	USERS 50
#define	USERFILE "/etc/utmp"
#include <stdio.h>
#include <utmp.h>
#include "../cmd_err.h"

char	message[3000];		/*	Message array			*/
int	messlen;		/*	Message length			*/
struct	utmp	users[USERS];	/*	User terminal & name info	*/

main(argc, argv)
int argc;
char	*argv[];
{
register chr, i;
register struct	utmp	*userptr;
FILE	*stream;

	/*	Initiate prompt string					*/
	PRMPT = *argv;

	++argv;
	--argc;

	if ((stream = fopen(USERFILE, "r")) == NULL) {
		fprintf(stderr, NOOPEN, USERFILE);
		exit(1);
	}

	fread((char *)users, sizeof(struct utmp), USERS, stream);
	fclose(stream);

	if (argc && (stream = fopen(*argv, "r")) == NULL) {
			fprintf(stderr, NOOPEN, *argv);
			exit(1);
	} else
		stream = stdin;

	while ((chr = getc(stream)) != EOF)
		message[messlen++] = chr;
	fclose(stream);

	while (i < USERS)
		if ((userptr = &users[i++])->ut_name[0]) {
			sleep(1);
			sendmsg(userptr->ut_line);
		}
	exit(0);
}

sendmsg(tty)
char	*tty;
{
register process;
char	device[50], buffer[BUFSIZ];
FILE	*stream;

	process = fork();
	if (process == -1) {
		fprintf(stderr, AGAIN);
		return;
	}
	if (process != 0)	/*	Is it the parent		*/
		return;

	strcpy(device, "/dev/");
	strcat(device, tty);

	if ((stream = fopen(device, "w")) == NULL) {
		fprintf(stderr, NOOPEN, device);
		exit(1);
	}
	setbuf(stream, buffer);	/*	Assign buffer to the stream	*/
	fprintf(stream, BRDCAST);
	fwrite(message, messlen, 1, stream);
	exit(0);
}

						