/*
 *	change date	SYSTEM 3 compatible
 *
 *	Copyright (c) DIAB T{by 1984
 *	Written by: Magnus Hedner
 *
 *	date [ -u ]   [ [ [ mm ] dd ] hhmm [ yy ] ]           [ +format ]
 *	date [ -u ]   [ [ [ [ yy ] mm ] dd ] hhmm [ .ss ] ]   [ +format ]
 *
 *	format:		result:
 *	%%		%
 *	%n		\n
 *	%t		\t
 *	%m		month - 01 to 12
 *	%d		day of month - 01 to 31
 *	%y		year mod 100 - 00 to 99
 *	%D		date as mm/dd/yy
 *	%H		hour - 00 to 23
 *	%M		minute - 00 to 59
 *	%S		second - 00 to 59
 *	%T		time as hh:mm:ss
 *	%g		timezone as GMT+/-nn:nn
 *	%j		julian date - 001 to 366
 *	%w		day of week - sunday = 0
 *	%a		abbreviated weekday - Sun to Sat
 *	%h		abbreviated month - Jan to Dec
 *	%r		time as hh:mm:ss AM/PM
 *	%z		abbreviated time zone name
 *	%G		use GMT from now on
 *	%L		use local time from now on
 */
/*eject*/
#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <time.h>
#include <utmp.h>
#include "../cmd.h"
#include "../cmd_err.h"
#define BUFSZ 200
#define MAXTM 30

/*	External functions						*/
/*	==================						*/
struct	timeb tp = {0};		/*	Time info structure		*/
char	*asctime();		/*	Convert time-count to string	*/
struct	tm *gmtime();		/*	Convert time-count to GMT	*/
struct	tm *localtime();	/*	Convert time-count to local time*/
struct	tm timegmt = {0};	/*	Holder for result of gmtime()	*/
struct	tm timeloc = {0};	/*	Holder for result of localtime()*/
struct	tm *mtime;		/*	Pointer to selected timestruc.	*/

/*	External variables						*/
/*	==================						*/
extern	char *tzname;		/*	Time zone names			*/

/*	Global variables						*/
/*	================						*/
static	int	monsize[12] = {
	31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static	struct	utmp	wtmp_bef = { "|", "", 0 };
static	struct	utmp	wtmp_aft = { "{", "", 0 };
static	char	format[] = "+%a %h %d %T %z 19%y";	/*	Default	*/
/*eject*/
/*	Main program							*/
/*	============							*/
main(argc, argv)
int argc;
char *argv[];
{
	int	year = -1, mon = -1, day = -1, hour = -1, min = -1, sec = 0;
	int	i, filedsc, result = 0, pos = 0, oldver = 0, uflg = FALSE;
	char	timestr[MAXTM], buf[BUFSZ], *a, *b;
	time_t	timecnt;

	/*	Initiate prompt string					*/
	PRMPT = *argv;

	--argc;
	++argv;

	if( argc && *argv && !strcmp(*argv, "-u")) {/*	Skip "old" switch	*/
			--argc;
			++argv;
			uflg = TRUE;
		}

	ftime(&tp);
	if(uflg)	mtime = gmtime(&tp.time);
	else		mtime = localtime(&tp.time);
	strncpy(timestr, asctime(mtime), MAXTM);
	/*eject*/
	if(*argv && **argv != '+') {	/*	Set time		*/
		for(i = 0; i < strlen(*argv); i++) {
			buf[i] = argv[0][i];
			if((i==4 || i==6 || i==8 || i==10) && buf[i] == '.') {
				if(oldver)
					result++;
				else
					oldver = i;
			} else
				if(!isdigit(buf[i]))
					result++;
		}
		buf[i] = '\0';

		if((oldver && oldver != (i-3)) || result ||
		   (i != 4 && (i < 6 || i > 11) && i != 13)) {
			fprintf(stderr, BADCONV);
			exit(1);
		}

		if(i & 1) {
			sec = atoi(&buf[i - 2]);
			i -= 3;
			buf[i] = '\0';
		}
		/*eject*/
		if(i == 10) {
			if(oldver || *buf > '1') { /*	Move year	*/
				buf[10] = buf[0];
				buf[11] = buf[1];
				buf[12] = '\0';
				for(a = buf; (*a = a[2]); a++) ;
			}
			year = atoi(&buf[8]);
			i -= 2;
		} else
			year = mtime->tm_year;

		buf[8] = '\0';

		min = atoi(&buf[i - 2]);

		buf[i - 2] = '\0';

		hour = atoi(&buf[i - 4]);

		buf[i - 4] = '\0';

		if(i >= 6) {
			day = atoi(&buf[i - 6]);
			buf[i - 6] = '\0';
		} else
			day = mtime->tm_mday;

		if(i >= 8)
			mon = atoi(buf);
		else
			mon = mtime->tm_mon;
		/*eject*/
		if(hour == 24) hour = 0;


		if(mon  < 1 || mon  > 12 ||
		   day  < 1 || day  > monsize[mon-1] ||
		   (day == 29 && mon == 2 && dysize(year) == 365) ||
		   hour < 0 || hour > 23 ||
		   min  < 0 || min  > 59 ||
		   sec  < 0 || sec  > 59) {
			fprintf(stderr, BADCONV);
			exit(1);
		}

		/*	Count no of sec's from Jan 1, 1970		*/
		timecnt = 0;
		year += 1900;
		if(year < 1970) year += 100;
		if(dysize(year) == 365 && mon > 2) timecnt--;
		while(--year > 1969) timecnt += dysize(year);

		while(--mon) timecnt += monsize[mon - 1];

		timecnt += day - 1;

		timecnt = 24 * timecnt + hour;
		timecnt = 60 * timecnt + min;
		if(!uflg) {
			timecnt += tp.timezone;
			if(tp.dstflag) timecnt -= 60;
		}
		timecnt = 60 * timecnt + sec;
#ifdef DEBUG
		fprintf(stderr,"%s: y:%d m:%d d:%d h:%d m:%d s:%d t:%ld\n",
			PRMPT, year, mon, day, hour, min, sec, timecnt);
#endif
		/*eject*/
		/*	Get time count before change of date		*/
		time(&wtmp_bef.ut_time);

		/*	Try to change the date				*/
		if(stime(&timecnt) == -1) {
			fprintf(stderr, NOPERM);
			exit(1);
		}
		if((filedsc = open (WTMP_FILE, 1)) == -1) {
/*			fprintf(stderr, NOOPEN, WTMP_FILE);
			result++;*/
		} else {
			/*	Get time count after change		*/
			time(&wtmp_aft.ut_time);

			/*	Append to file				*/
			lseek(filedsc, 0L, 2);

			/*	Try to write the two records		*/
			if(write(filedsc, &wtmp_bef, sizeof(struct utmp)) !=
			sizeof(struct utmp) ||
			write(filedsc, &wtmp_aft, sizeof(struct utmp)) !=
			sizeof(struct utmp)) {
				fprintf(stderr, WRERR, WTMP_FILE);
				result++;
			}
			close(filedsc);
		}
		argv++;
		argc--;
	}
	/*eject*/
	if(*argv) {
		if((**argv != '+') || argv[1]) {	/*	Unknown	*/
			fprintf(stderr, "usage: %s [mmddhhmm[yy]] [+format]",
			PRMPT);
			exit(1);
		}
	} else {
		if(wtmp_bef.ut_time)	/*	Date is set		*/
			exit(result);	/*	No arg's left		*/
		else
			*argv = format;
	}

	ftime(&tp);
	mtime = localtime(&tp.time);
	for(a = &timeloc,b = mtime,i = 0;i++ < sizeof(struct tm);*a++ = *b++);
	mtime = gmtime(&tp.time);
	for(a = &timegmt,b = mtime,i = 0;i++ < sizeof(struct tm);*a++ = *b++);
	if(uflg){
		mtime = &timegmt;
	} else {
		mtime = &timeloc;
	}
	strncpy(timestr, asctime(&timegmt), MAXTM);
	/*eject*/
	while(*++*argv && pos < BUFSZ - 13)
		if(**argv != '%')
			buf[pos++] = **argv;
		else
			switch(*++*argv) {
			case '%':/*	Doubled				*/
				buf[pos++] = '%';
				break;
			case 'n':/*	Insert a newline		*/
				buf[pos++] = '\n';
				break;
			case 't':/*	Insert a tab			*/
				buf[pos++] = '\t';
				break;
			case 'm':/*	Month - 01 to 12		*/
				sprintf(&buf[pos], "%02d", mtime->tm_mon+1);
				pos += 2;
				break;
			case 'd':/*	Day of month - 01 to 31		*/
				sprintf(&buf[pos], "%02d", mtime->tm_mday);
				pos += 2;
				break;
			case 'y':/*	Year mod 100			*/
				sprintf(&buf[pos], "%02d", mtime->tm_year);
				pos += 2;
				break;
			case 'D':/*	Date as mm/dd/yy		*/
				sprintf(&buf[pos], "%02d/%02d/%02d",
				mtime->tm_mon+1, mtime->tm_mday,
				mtime->tm_year);
				pos += 8;
				break;
			case 'H':/*	Hour - 00 to 23			*/
				sprintf(&buf[pos], "%02d", mtime->tm_hour);
				pos += 2;
				break;
			case 'M':/*	Minute - 00 to 59		*/
				sprintf(&buf[pos], "%02d", mtime->tm_min);
				pos += 2;
				break;
			case 'S':/*	Second - 00 to 59		*/
				sprintf(&buf[pos], "%02d", mtime->tm_sec);
				pos += 2;
				break;
			case 'T':/*	Time as hh:mm:ss		*/
				sprintf(&buf[pos], "%02d:%02d:%02d",
				mtime->tm_hour, mtime->tm_min,
				mtime->tm_sec);
				pos += 8;
				break;
			case 'g':/*	Timezone - GMT+/-nn:nn		*/
				sprintf(&buf[pos], "GMT%c%02d:%02d",
				(tp.timezone >= 0 ? '-' : '+'),
				abs(tp.timezone) / 60,
				abs(tp.timezone) % 60);
				pos += 9;
				break;
			case 'j':/*	Julian date - 001 to 366	*/
				sprintf(&buf[pos], "%03d", mtime->tm_yday+1);
				pos += 3;
				break;
			case 'w':/*	Day of week - Sunday = 0	*/
				sprintf(&buf[pos], "%01d", mtime->tm_wday);
				pos++;
				break;
			case 'a':/*	Abbreviated weekday - Sun to Sat*/
				sprintf(&buf[pos], "%.3s", timestr);
				pos += 3;
				break;
			case 'h':/*	Abbreviated month - Jan to Dec	*/
				sprintf(&buf[pos], "%.3s", &timestr[4]);
				pos += 3;
				break;
			case 'r':/*	Time in AM/PM notation		*/
				i = mtime->tm_hour % 12;
				if(i == 0) i = 12;
				sprintf(&buf[pos], "%02d:%02d:%02d %cM",
				i, mtime->tm_min, mtime->tm_sec,
				(mtime->tm_hour >= 12 ? 'P' : 'A'));
				pos += 11;
				break;
			case 'z':/*	Time zone name			*/
				if((a = &tzname[timeloc.tm_isdst])) {
					sprintf(&buf[pos], "%s", a);
					pos += strlen(a);
				}
				break;
			case 'G':/*	Time measured in GMT		*/
				mtime = &timegmt;
				break;
			case 'L':/*	Time measured in local time	*/
				mtime = &timeloc;
				break;
			default:
				fprintf(stderr, BADFORM, *argv);
				exit(1);
			}

	buf[pos++] = '\n';
	buf[pos]   = '\0';
	printf("%s", buf);

	exit(result);
}
