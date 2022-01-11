/*
	NAME
		ps - process status

	SYNOPSIS
		ps [lax]

	DESCRIPTION
		Ps prints certain indicia about active processes. The a option
		asks for information about all processes with terminals
		(ordinarily only one's own processes are displayed); x asks
		even about processes with no terminals; l asks for long listing.
		The long listing is columnar and contains

		UID	The user id of the process owner
		TTY	Controlling terminal
		PID	The process id of the process
		PID	The process id of the parent process
		STATE	Flags associated with the process on format xxx:s
			T - Process is being traced by other process
			L - Process is locked in core
			S - Process is swapped

			The s-flag holds the stat of the process:
			R - Process is runnable
			S - Process is sleeping (in wait state)
			Z - Intermidiate (zombi state)
			P - Process is stopped

		PRI	The priority of the process; high numbers mean low priority
		SIZE	Size of process in core on format 'N1 + N2'.
			N1 holds size of sharable code.
			N2 holds size of non-sharable code.
		UTIME	The cumulative user time of the process
		STIME	The cumulative system time of the process
		CMD	The command typed at invokation of the process

	The short listing contains the fields UID,TTY,PID,PPID and CMD

 */
/*eject*/
#include <sys/types.h>
#include <dnix/fcodes.h>
#include <dnix/pstat.h>
#include <dnix/errno.h>
#include <stdio.h>

struct psinfo tcbinfo;	/* holds info for each process */
int olduid = -1;
char **usrinfo;

char bighead[] = {
"UID    TTY     PID   PPID  STATE PRI   SIZE   UTIME STIME CMD"
};
char smallhead[] = {
"UID    TTY     PID   PPID  CMD"
};

char buffer[BUFSIZ];

main(argc,argv)
register char **argv;
{
	register char *cpnt;
	register int lflag,aflag,xflag,rstat,ownid,i;

	setbuf(stdout,buffer);
	lflag = aflag = xflag = 0;
	if(argc>1){
		cpnt = argv[1];
		while (*cpnt){
			switch (*cpnt++){
			case 'a':
				aflag++;
				break;
			case 'l':
				lflag++;
				break;
			case 'x':
				xflag++;
				break;
			}
		}
	}
	ownid = getuid();	/* get own user id */
	fprintf(stdout,"%s\n",(lflag ? bighead : smallhead) );
	for(i = 0;;i++){
		rstat = dnix(F_PSTAT,&tcbinfo,i);
		if(rstat < 0){
			if(rstat == ESRCH){
				/* No such process - continue */
				continue;
			}
			fclose(stdout);		/* Empty buffer */
			exit(0);		/* No more processes */
		}
		if( (xflag) || (tcbinfo.ps_uid == ownid) || 
		    (aflag && (tcbinfo.ps_pgrp != 0)) ){
			/* List info for this process */
			pslist(lflag);
		}
	}
}
/*eject*/
pslist(longlist)
int longlist;
{
	char flags[6];

	if(longlist){
		psfirst();
		if(tcbinfo.ps_flags & PS_FLOCK){
			flags[0] = 'L';
		} else {
			flags[0] = '-';
		}
		if(tcbinfo.ps_flags & PS_FSTRC){
			flags[1] = 'T';
		} else {
			flags[1] = '-';
		}
		if(tcbinfo.ps_flags & (PS_FSWOUT + PS_FSWPED + PS_FSWIN)){
			flags[2] = 'S';
		} else {
			flags[2] = '-';
		}
		flags[3] = ':';
		switch (tcbinfo.ps_state) {
		case PS_SSLEEP: flags[4] = 'S';break;
		case PS_SRUN  : flags[4] = 'R';break;
		case PS_SSZOMB: flags[4] = 'Z';break;
		case PS_SSTOP : flags[4] = 'P';break;
		default      : flags[4] = '-';break;
		}
		flags[5] = '\000';
		fprintf(stdout,"%-5s ",flags);
		fprintf(stdout,"%-3d ",tcbinfo.ps_dpri);
		fprintf(stdout,"%3d+%3dk ",(tcbinfo.ps_puasd.pasd_sz/1024),
				(tcbinfo.ps_imasd.pasd_sz +
				tcbinfo.ps_stasd.pasd_sz +
				tcbinfo.ps_swasd.pasd_sz) /1024);
		prtim(tcbinfo.ps_utime);
		prtim(tcbinfo.ps_stime);
		pscmd();
	} else {
		psfirst();
		pscmd();
	}
	fprintf(stdout,"\n");
	return;
}

prtim(ti)
register unsigned long ti;
{
	if(ti < 3600 ){
		fprintf(stdout,"%2d.%02d ",ti / 60,ti % 60);
	} else {
		ti /= 60;
		if(ti >= 6000){
			ti = 5999;
		}
		fprintf(stdout,"%2d:%02d ",ti / 60,ti % 60);
	}
}

psfirst()
{
	register int i;

	if(tcbinfo.ps_uid != olduid){
		/* Get symbolic user id if diffrent from previous */
		usrinfo = (char **)getpwuid(tcbinfo.ps_uid);
		olduid = tcbinfo.ps_uid;
	}
	if(usrinfo == 0){
		fprintf(stdout,"%-6d ",olduid);
	} else {
		*usrinfo[6] = '\000';
		fprintf(stdout,"%-6s ",*usrinfo);
	}
	if(major(tcbinfo.ps_ttyd) != 1){
		fprintf(stdout,"        ");
	} else {
		i = minor(tcbinfo.ps_ttyd) & 127;
		if(i == 1){
			fprintf(stdout,"console ");
		} else {
			fprintf(stdout,"tty%d%d   ",i/10,i % 10);
		}
	}
	fprintf(stdout,"%-5d ",tcbinfo.ps_pid);
	fprintf(stdout,"%-5d ",tcbinfo.ps_ppid);
}


pscmd()
{
	if(tcbinfo.ps_pid == 0){
		fprintf(stdout,"Swapper");
	} else {
	/*
		tcbinfo.ps_spar[20] = '\000';
	*/
		if(tcbinfo.ps_flags & (PS_FSWOUT + PS_FSWPED + PS_FSWIN)){
			fprintf(stdout,"Process is swapped");
		} else {
			fprintf(stdout,"%s",tcbinfo.ps_spar);
		}
	}
}
