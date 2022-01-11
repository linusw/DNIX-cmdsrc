/*	Function printing out information about a file - fileinfo	*/
/*	=========================================================	*/
/*									*/
/*	Copyright (C) DIAB T{by 1983					*/
/*	Written by: Per-Arne Svensson 					*/

#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>


fileinfo(name, statptr, inodflg, sizeflg, longflg, grpflg, acctime, cretime)
char	*name;
struct	stat *statptr;
{
unsigned short	ftype;
char		*ctime(), *timestrg;
time_t		time(), filetime;
time_t		bef6mon;	/*	Time 6 months ago		*/


	if (inodflg) 		/*	Test if inode-number		*/
		printf("%5u ", statptr->st_ino);
	if (sizeflg)		/*	Test if size in blocks		*/
		if ((ftype = statptr->st_mode & S_IFMT) == S_IFCHR
					|| ftype == S_IFBLK)
		printf("%4D ", ((statptr->st_rdev+511)>>9));
	else
		printf("%4D ", ((statptr->st_size+511)>>9));
	if (longflg) {		/*	Test if long listing		*/
		printf("%s ", filemode(statptr));
		/*	Print number of links to the file		*/
		printf("%2d ", statptr->st_nlink);
		/*	Print user or group name			*/
		printf("%-6.6s ", IDname(grpflg ? statptr->st_gid : 
			statptr->st_uid, grpflg));
		/*	Print file size					*/
		if ((ftype = statptr->st_mode & S_IFMT) == S_IFCHR
					|| ftype == S_IFBLK)
			printf("%3d,%3d ", major((int)statptr->st_rdev)
					, minor((int)statptr->st_rdev));
		else
			printf("%7ld ", statptr->st_size);

		/*	Print file-time					*/

		/*	Set bef6mon to the time in seconds 6 months ago	*/
		bef6mon = time((long *)0)-(6L * 30L * 24L * 60L * 60L);

		if (acctime)
			filetime = statptr->st_atime;
		else if (cretime)
			filetime = statptr->st_ctime;
		else
			filetime = statptr->st_mtime;
		/*	Convert time to an ascii-string 		*/
		timestrg = ctime(&filetime);
		if (filetime > bef6mon)
			printf("%-12.12s ", timestrg+4);
		else
			printf("%-7.7s %-4.4s ", timestrg+4, timestrg+20);
	}

	/*	Print file-name						*/

	printf("%s\n", name);

	return;
}

