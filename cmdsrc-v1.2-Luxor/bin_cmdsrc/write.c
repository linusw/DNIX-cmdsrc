/*  Write message to another user  */

#include <stdio.h>
#include <utmp.h>
#include <signal.h>

char	*ttyname();

char	*userid;
char	usertty[80] = "/dev/" ;
int	usercount;
char	ownid[20] = "?????" ;
char	*owntty;
FILE	*fp;
struct utmp	utmpbuf;

int	timeout();

main(parcount,param)

int	parcount;
char	*param[];

{
	if (parcount<2) {
		printf("usage: write user [ttyname]\n");
		exit(1);
	}
	userid=param[1];
	if (parcount!=2) {
		strcat(usertty,param[2]);
	}
	owntty=ttyname(0);
	if (owntty++ == NULL)
		owntty="/unknown tty";
	while ( *(owntty++) != '/' )
		;
	if ((fp=fopen("/etc/utmp","r")) == NULL ) {
		printf("Can't open file: /etc/utmp\n");
		if (parcount == 2)
			exit(1);
	}
	while (read(fileno(fp),&utmpbuf,sizeof(utmpbuf)) > 0) {
		if ((strcmp(userid,"-") == 0) || (strcmp(userid,utmpbuf.ut_name) == 0 )) { 
			if (parcount == 2) {
				if (++usercount == 1) 
					strcat(usertty,utmpbuf.ut_line);
			}
			else {
				if (strcmp(usertty+5,utmpbuf.ut_line) == 0)
					usercount++;
			}
		}
		if (strcmp(owntty,utmpbuf.ut_line) == 0)
			strcpy(ownid,utmpbuf.ut_name);
	}
	fclose(fp);
	if (usercount == 0) {
		if (parcount == 2)
			printf("%s not logged in\n",userid);
		else
			printf("%s not on that tty\n",userid);
		exit(1);
	}
	if (usercount > 1 && parcount == 2) {
		printf("%s logged in more than once\nwriting to %s\n",userid,usertty+5);
	}
	signal(SIGALRM,timeout);
	alarm(5);
	if ((fp=fopen(usertty,"w")) != NULL) {
		int	cnt;
		char	buffer[256];
		alarm(0);
		fprintf(fp,"Message from %s %s ...\n",ownid,owntty);
		fflush(fp);
		do {
			cnt=read(0,buffer,256);
			if (cnt) {
				if (buffer[0] == '!') {
					buffer[cnt]='\0';
					system(buffer+1);
					printf("!\n");
				}
				else {
					write(fileno(fp),buffer,cnt);
				}
			}
		} while (cnt);
		fprintf(fp,"EOF\n");
	}
	else { 
		printf("Permission denied\n");
	}
}

timeout()
{
	printf("Timeout when opening his tty\n");
}
