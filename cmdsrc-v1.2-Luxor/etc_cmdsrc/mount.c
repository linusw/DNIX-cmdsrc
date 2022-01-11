/*
 * Copyright (C) Dataindustrier AB, 1983
 */
/*
 *	Mount/unmount command
 *	Invoked as
 *	mount handler device directory params...
 *	or
 *	unmount device
 *
 *
 *	MODIFICATIONS:
 *
 *	Changed "svc" to "dnix" to be consistent.	84-05-21 (paf)
 *
 *	Added wait for file handler error status before update
 *	of etc/mtab at mount.		84-05-11 LA
 *
 *	Changed parameters at mount to be compatible with unix mount.
 *	Now invoked as: MOUNT device directory params or
 *			MOUNT handler device directory params
 *	Handler is read from disk to be mounted and started as /hnd/handler
 *	if not specified.		84-05-24 LA
 *
 *	Added conditional for rindex as strrchr. 84-11-06 paf.
 */

#include <stdio.h>
#include <signal.h>
#include <fcodes.h>

#define	NMOUNT	16
#define	NAMSIZ	32
#define WAIT_FHND_ERR	3
#define HANDLER 2048

#ifdef	M_V7
#define	strrchr	rindex
#endif

struct mtab {
	char	pid[NAMSIZ];
	char	hnd[NAMSIZ];
	char	file[NAMSIZ];
	char	spec[NAMSIZ];
} mtab[NMOUNT];

char name[] = "/etc/mtab";
char dskhnd[] = "/hnd/0123456789abcdef";
extern char *strrchr();
extern int errno;
int clock();
int no_timeout = 1;	
char *fhnd,*fdev,*fdir;
char **fpar;
char buf[512];


main(argc, argv)
char **argv;
{
	char *t;

	dnix((short)F_SPG,(int)0);
	if((t = strrchr(argv[0],'/')) == 0)
		t = argv[0];
	else
		++t;
	if(*t == 'u')
		unmnt(argc,argv);
	else
		mnt(argc,argv);
}

mnt(argc,argv)
char **argv;
{
	int pid;
	register struct mtab *mp;
	register char *np;
	int mf,dev;
	int errcode;

	mf = open(name, 0);
	read(mf, (char *)mtab, NMOUNT * sizeof(struct mtab));
	if (argc==1) {
		for (mp = mtab; mp < &mtab[NMOUNT]; mp++)
			if (mp->file[0])
				printf("PID %s HND %s DEV %s ON %s\n", mp->pid,mp->hnd,mp->spec, mp->file);
		exit(0);
	}
	if(argc < 3) {
		fprintf(stderr,
	"mount: invalid args, use - mount [fhnd] device directory params\n");
		exit(1);
	}
/*
 *	If argc = 3 or if argv[1] does not have execute previliges then
 *	asume no handler is specified - read handler from disk.
 */
	if( (argc == 3) || (access(argv[1],1) != 0) ){
		char *s,*d;

		argv[0] = dskhnd;
		fhnd = argv[0]; fdev = argv[1]; fdir = argv[2]; fpar = argv;
		dev=open(fdev,0);	/* open device */
		if( dev < 0 ){
			fprintf(stderr,"mount: Cannot open %s\n",fdev);
			exit(1);
		}
		lseek(dev,HANDLER,(unsigned long)0);
		if(read(dev,buf,512) < 0){
			fprintf(stderr,"mount: Cannot read %s\n",fdev);
			exit(1);
		}
		buf[15] = '\000';
		s = buf; d = &dskhnd[5];
		while(*d++ = *s++);
	} else {
		fhnd = argv[1]; fdev = argv[2]; fdir = argv[3]; fpar = argv+1;
	}
	pid = fork();
	if(pid == 0){
		execv(fhnd,fpar);
		fprintf(stderr,
		"mount: Cannot execute %s, status=%d\n",fhnd,errno);
		exit(1);
	}
/*
 *	Set up catching of alarm, set alarm.
 *	Wait for alarm or termination of file handler
 *
 */
	signal(SIGALRM,clock);
	alarm(WAIT_FHND_ERR);
	wait();
	if(no_timeout){
		/* No timout - file handler has terminated */
		fprintf(stderr,"mount: Unable to mount device!\n");
		exit(1);
	}
	np = fdev;
	while(*np++)
		;
	np--;
	while(*--np == '/')
		*np = '\0';
	while(np > fdev && *--np != '/')
		;
	if(*np == '/')
		np++;
	fdev = np;
	for (mp = mtab; mp < &mtab[NMOUNT]; mp++) {
		if (mp->file[0] == 0) {
			sprintf(mp->pid,"%d",pid);
			for (np = mp->hnd; np < &mp->hnd[NAMSIZ-1];)
				if ((*np++ = *fhnd++) == 0)
					fhnd--;
			for (np = mp->spec; np < &mp->spec[NAMSIZ-1];)
				if ((*np++ = *fdev++) == 0)
					fdev--;
			for (np = mp->file; np < &mp->file[NAMSIZ-1];)
				if ((*np++ = *fdir++) == 0)
					fdir--;
			mp = &mtab[NMOUNT];
			while ((--mp)->file[0] == 0);
			if((mf = creat(name, 0644)) < 0){
				fprintf(stderr,
				"mount: Cannot creat %s\n",name);
				exit(1);
			} else {
				write(mf, (char *)mtab, (mp-mtab+1)*sizeof(struct mtab));
			}
			exit(0);
		}
	}
	exit(0);
}
unmnt(argc, argv)
char **argv;
{
	int pid;
	register struct mtab *mp;
	register char *p1, *p2;
	int mf;

	sync();
	mf = open(name, 0);
	read(mf, (char *)mtab, NMOUNT * sizeof(struct mtab));
	if(argc != 2) {
		fprintf(stderr,
		"umount: Invalid args, use - umount dev\n");
		exit(1);
	}
	p1 = argv[1];
	while(*p1++)
		;
	p1--;
	while(*--p1 == '/')
		*p1 = '\0';
	while(p1 > argv[1] && *--p1 != '/')
		;
	if(*p1 == '/')
		p1++;
	argv[1] = p1;
	for (mp = mtab; mp < &mtab[NMOUNT]; mp++) {
		p1 = argv[1];
		p2 = &mp->spec[0];
		while (*p1++ == *p2)
			if (*p2++ == 0) {
				sscanf(mp->pid,"%d",&pid);
				if(pid > 0)
					kill(pid,SIGTERM);
					sleep(WAIT_FHND_ERR);
				for (p1 = (char *)mp ; p1 < (char *)(mp + 1);)
					*p1++ = 0;
				mp = &mtab[NMOUNT];
				while (mp>mtab && (--mp)->file[0] == 0)	/*M000*/
					;				/*M000*/
				if((mf = creat(name, 0644)) < 0){
					fprintf(stderr,
					"umount: Cannot create %s\n",name);
					exit(1);
				} else {
					write(mf, (char *)mtab, (mp-mtab+1)*sizeof(struct mtab));
				}
				exit(0);
			}
	}
	printf(stderr,"umount: %s not in mount table\n", argv[1]);
	exit(1);
}


clock()
{
	no_timeout = 0;	/* mark timeout */
	return;
}
