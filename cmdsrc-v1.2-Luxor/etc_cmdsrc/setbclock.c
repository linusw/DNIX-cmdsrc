/*
 *	set battery clock
 *
 *	Copyright (c) DIAB T{by 1984
 *	Written by: Magnus Hedner
 */

/*
 *	/etc/setbclock yymmddhhmm[.ss]
 */

#include <stdio.h>
#include "../cmd.h"
#include "../cmd_err.h"

static int monsize[12] = {
	31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static char clock[] = "/dev/bclock";

main(argc, argv)
int argc;
char **argv;
{
	int year, mon, date, hour, min, sec, bclock, i;
	char buf[15];

	/*	Initiate prompt string					*/
	PRMPT = *argv;
	argv++;
	argc--;

	if(argc == 1 && *argv) {	/*	set time		*/
		for(i = 0;i < 15 && (buf[i] = (*argv)[i]); i++) ;
		if(i != 13 && i != 10) goto error;
		if(i == 10)
			strcpy(&buf[10], ".00");
		if(sscanf(buf, "%2d%2d%2d%2d%2d.%2d", &year, &mon, &date,
		   &hour, &min, &sec) != 6)
			goto error;

		if(mon  < 1 || mon  > 12 ||
		   date < 1 || date > monsize[mon - 1] ||
		   (date == 29 && mon == 2 && dysize(year) == 365) ||
		   hour < 0 || hour > 23 ||
		   min  < 0 || min  > 59 ||
		   sec  < 0 || sec  > 59)
			goto error;

		if((bclock = open(clock, 1)) == -1) {
			fprintf(stderr, NOOPEN, clock);
			exit(1);
		}

		write(bclock, buf, 13);

		exit(0);	/*	Success!			*/
	}

error:
	fprintf(stderr, "usage:  %s yymmddhhmm[.ss]\n", PRMPT);
	exit(1);
}
