/*
*	Tape archive program.
*	---------------------
*
*	Copyright DataIndustrier DIAB AB.
*
*	Additional features.
*
*	840829 CE.
*	When dumping, stat used instead of fstat to avoid
*	side effects on devices.
*	Made 'm' option to preserve accessed time in create mode.
*	Corrected bug in setting accessed time in extract mode.
*	Block tapes can be updated, need not to be 512 bytes block.
*	Speed option added ('s'), means that io is done with
*	no wait on tape device.
*/
/*eject*/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <stdio.h>
#include <signal.h>
#include <dnix/fcodes.h>
#define BYTE_1 char
#define BYTE_4 int
#define REG register
#define EXT extern
#include "tar.h"

EXT int io(), notsame(), readdir(), forwfile(), closevol();
EXT int tapepadd(), longt(), checkdir();
EXT int onintr(), onquit(), onhup();
EXT int tomodes(), calcchksum(), checkw(), askuser(), checkupdate();
EXT int telldef(), newvol(), endprog(), prefix(), getwd();
EXT strncpy(), diskseek(), iowrtape(), iordtape();
EXT BYTE_1 *prmode();
EXT daddr_t high;

struct stat stbuf;				/* saved stat for file. */
struct linkbuf *ihead;				/* root pointer for link info. */
union hblock dirinfo;				/* saved header block. */

int	copt;					/* create new tape. */
int	mopt;					/* current time as mod time. */
int	ropt;					/* replace, append files at end. */
int	sopt;					/* high speed option. */
int	topt;					/* list a table over all files. */
int	vopt;					/* verbose option. */
int	wopt;					/* interactive mode. */
int	xopt;					/* extract named files from tape. */
int	yopt;					/* keep on reading until dir info found. */
int	tapelu = -1;				/* lu no for tape device. */
int	term;					/* flag set for termination. */
int	chksum;					/* calculated checksum. */
int	tblkpos;				/* current block pos on tape. */
int	suflag;					/* su flag. */
int	cblkno, linkerrok, first;
int	freemem = 1;
int	tblksz = 1;				/* default blockfactor. */

FILE	*tfile;
BYTE_1	tname[] = "/tmp/tarXXXXXX";
BYTE_1	*tapefile;
BYTE_1	magtape[]	= "/dev/mt1";

BYTE_1	*malloc();
BYTE_1	*sprintf();

int	mulvol;					/* multiple volumes. */
BYTE_4	tblkmax;				/* max no of blks per volume. */
long	atol();
int	NotTape;				/* if tape is a disk. */
int	tapewflg;
int	extno;					/* number of extents. */
int	extotal;				/* total no of extents. */
BYTE_4	efsize;					/* size of entire file. */
FILE *Logfile = stdout;				/* default log file. */
/*eject*/
/*
*	Tar, Tape archive program.
*	--------------------------
*/
main(argc, argv)
int	argc;
BYTE_1	*argv[];
{
	BYTE_1 *s;
	int tapeextr(), tapecreate(), tapelist();
	int fc, lu;
	int (*hnd)();				/* function handler. */

	if (argc < 2)
		telldef();

	tfile = NULL;
	tapefile =  magtape;
	argv[argc] = 0;
	argv++;
	suflag = 0;
	tblksz = MXBLKNO;			/* set largest buffer used. */
	if(getuid() == 0) {
		suflag++;
	}
	for (s = *argv++; *s; s++) 
		switch(*s) {
		case 'f':
			if((tapefile = *argv++) == 0) {
				goto badf;
			}
			if (tblksz == 1)
				tblksz = 0;
			break;
		case 'c':
			copt++;
			ropt++;
			sopt++;			/* speedy gonzales if 'c' */
			break;
		case 'u':
			mktemp(tname);
			if ((tfile = fopen(tname, "w")) == NULL) {
				fprintf(stderr, "tar: cannot create temporary file (%s)\n", tname);
				endprog(1);
			}
			fprintf(tfile, "!!!!!/!/!/!/!/!/!/! 000\n");
		case 'r':
			ropt++;
			break;
		case 'v':
			vopt++;
			break;
		case 'w':
			wopt++;
			break;
		case 'x':
			xopt++;
			break;
		case 'y':
			yopt++;
			break;
		case 's':
			sopt++;
			break;
		case 't':
			topt++;
			break;
		case 'm':
			mopt++;
			break;
		case '-':
			break;
		case '0':
		case '1':
			magtape[7] = *s;
			tapefile = magtape;
			break;
		case 'b':
			if(*argv == 0) {
badb:
				fprintf(stderr, "tar: Invalid blockfactor.\n");
				endprog(1);
			}
			tblksz = atoi(*argv++);
			if(tblksz <= 0)
				goto badb;
			if(tblksz > MXBLKNO) {
				fprintf(stderr, "tar: Warning, exceeded standard block factor( <= 20): %d\n", tblksz);
			}
			break;
		case 'k':
			tblkmax = atol(*argv++);
			if(tblkmax < 250L) {
				fprintf(stderr, "tar: sizes below 250K not supported (%lu).\n", tblkmax);
				endprog(1);
			}
			tblkmax *= 1024L/BLKSIZ;
			mulvol++;
		case 'n':
			NotTape++;
			break;
		case 'l':
			linkerrok++;
			break;
		case 'V':
			fprintf(stderr, "tar R1.02\n");
			break;
		default:
			fprintf(stderr, "tar: Unknown option: %c, ignored\n", *s);
		}
/*eject*/
/*
*	Open input/output files.
*/
	if(ropt && (copt == 0) && sopt) {
		fprintf(stderr, "tar: Invalid to use 's' when using 'u' or 'r'\n");
		endprog(1);
	}
	hnd = 0;
	fc = F_OPEN;
	if(ropt) {				/* replace, create or update. */
		if (copt && tfile != NULL) {
			telldef();
			endprog(1);
		}
		if (signal(SIGINT, SIG_IGN) != SIG_IGN)
			signal(SIGINT, onintr);
		if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
			signal(SIGHUP, onhup);
		if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
			signal(SIGQUIT, onquit);
/*              if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
 *                      signal(SIGTERM, onterm);
 */
		lu = 2;				/* read write attributes. */
		if(strcmp(tapefile, "-") == 0) {
			if(copt == 0) {		/* must be able to create. */
				fprintf(stderr, "tar: can only create standard output archives\n");
				endprog(1);
			}
			fc = F_DLU;
			lu = 1;			/* std output. */
			Logfile = stderr;	/* change logfile to stderr. */
		}
		hnd = tapecreate;
	} else if(topt || xopt) {		/* table or xtract. */
		lu = 0;				/* read only attr. */
		if(strcmp(tapefile, "-") == 0) {
			fc = F_DLU;		/* duplicate lu. */
			lu = 0;			/* use stdin as input. */
			Logfile = stderr;
		}
		hnd = tapelist;
		if(xopt) {
			hnd = tapeextr;
		}
	}
	if(fc == F_DLU) {			/* if any std io, blk=512. */
		tblksz = 1;
	}
	if((tapelu = io(fc, lu, tapefile, tblksz*BLKSIZ)) < 0) {
		if(ropt && copt && (fc == F_OPEN)) {
			tapelu = io(F_CREATE, 0666, tapefile, tblksz*BLKSIZ);
		}
		if(tapelu < 0) {
badf:
			fprintf(stderr, "tar: cannot open %s\n", tapefile);
			endprog(1);
		}
	}
	if(hnd) {
		(*hnd)(argv);			/* execute req function. */
	} else {
		telldef();
	}
	endprog(0);
}
/*eject*/
/*
*	Create, update and append a tape.
*	---------------------------------
*/
tapecreate(argv)
BYTE_1	*argv[];
{
	REG BYTE_1 *s, *r;
	BYTE_1 wdir[60];
	BYTE_1 buf[200];

	if(copt == 0) {				/* update or replace. */
		while(readdir() > 0) {
			if(term)
				endprog(0);	/* time to terminate? */
			forwfile();		/* else skip this file. */
		}				/* loop to eot. */
		if(tfile != NULL) {		/* any temp file spec'ed? */
			sprintf(buf, "sort +0 -1 +1nr %s -o %s; awk '$1 != prev {print; prev=$1}' %s >%sX;mv %sX %s",
				tname, tname, tname, tname, tname, tname);
			fflush(tfile);
			system(buf);
			freopen(tname, "r", tfile);
			fstat(fileno(tfile), &stbuf);
			high = stbuf.st_size;
		}
		if(io(F_SEEK, tapelu, -BLKSIZ, 1) < 0) {	/* back over header. */
			fprintf(stderr, "tar: seek error on tape.\n");
			endprog(1);
		}
	}
/*eject*/
/*
*	correct position on tape found.
*/
	tapewflg = 1;
#ifdef DEBUG
	wdir[0] = '.';
	wdir[1] = 0;
#else
	getwd(wdir);				/* get current directory. */
#endif
	if(mulvol) {
		if(tblksz && (tblkmax%tblksz) != 0) {
			fprintf(stderr, "tar: Volume size not a multiple of block size\n");
			endprog(1);
		}
		tblkmax -= 2;			/* reserve space for padd data at end. */
		tblkmax -= tblksz;		/* make sure full last block. */
						/* so all blocks got same size. */
						/* just for mag tape driver. */
		if(vopt)
			fprintf(stderr, "Volume ends at %luK, blocking factor = %dK\n",
				K(tblkmax - 1), K(tblksz));
	}
	--argv;
	while (*++argv && ! term) {
		s = *argv;
		while(*s == ' ') s++;		/* skip any spaces. */
		*argv = s;
		if(*s != '\0') {		/* if not empty string */
			for(r = s; *r; r++) {	/* scan after last '/' */
				if (*r == '/')
					s = r;
			}
			if (s != *argv) {
				*s = '\0';
				if(chdir(*argv)) {	/* tune in cur directory. */
					fprintf(stderr, "tar: can't 'cd' to %s\n", *argv);
					continue;
				}
				*s = '/';
				s++;
			}
			putfile(*argv, s);
			if(chdir(wdir)) {
				fprintf(stderr, "tar: fatal 'cd %s' error, abort\n", wdir);
				endprog(3);
			}
		}
	}
	closevol();
	if (linkerrok == 1)
		for (; ihead != NULL; ihead = ihead->nextp)
			if (ihead->count != 0)
				fprintf(stderr, "tar: Missing links to %s\n", ihead->pathname);
}
/*eject*/
/*
*	Put file.
*	---------
*
*	writes a complete file onto tape.
*/
putfile(longname, shortname)
BYTE_1 *longname;
BYTE_1 *shortname;
{
	int inlu;
	BYTE_4 noblks;
	BYTE_1 buf[BLKSIZ];			/* buffer to pack name in. */
	REG BYTE_1 *s, *r;			/* scratch variables. */
	REG int i, j;				/* scratch vars. */
	struct direct dirbuf;
	struct stat sb;				/* temp used with mopt. */
	time_t timep[2];
	int maxbsz;

	if(stat(shortname,&stbuf) < 0) {	/* find out if directory */
		fprintf(stderr,"tar: %s: cannot stat file\n", longname);
		return;
	}
	if(((stbuf.st_mode & S_IFMT) != S_IFDIR) &&
		((stbuf.st_mode & S_IFMT) != S_IFREG)) {
		fprintf(stderr, "tar: %s is not a file. Not dumped\n", longname);
		return;
	}
	if((inlu = open(shortname, 0)) < 0) {
		fprintf(stderr, "tar: %s: cannot open file\n", longname);
		return;
	}
	fstat(inlu, &stbuf);
	if (tfile != NULL && checkupdate(longname) == 0) {
		close(inlu);
		return;
	}
	if (checkw('r', longname) == 0) {
		close(inlu);
		return;
	}

/*eject*/
/*
*	handle directory.
*/
	if ((stbuf.st_mode & S_IFMT) == S_IFDIR) {
		for (i = 0, s = buf; *s++ = longname[i++];);
		*--s = '/';
		s++;
		i = 0;
		if(chdir(shortname)) {
			fprintf(stderr, "tar: can't 'cd' to %s.\n", longname);
			close(inlu);
			return;
		}
		while (read(inlu, (char *)&dirbuf, sizeof(dirbuf)) > 0 && !term) {
			if(dirbuf.d_ino == 0) {	/* read one entry, deleted file? */
				i++;
				continue;
			}
			if (strcmp(".", dirbuf.d_name) == 0 ||
				(dirbuf.d_name[0] == '.' &&
				dirbuf.d_name[1] == '.')) {
				i++;
				continue;
			}
			r = s;
			for (j=0; j < DIRSIZ; j++)
				*r++ = dirbuf.d_name[j];
			*r = '\0';
			close(inlu);
			putfile(buf, s);
			inlu = open(".", 0);
			i++;
			lseek(inlu, (long) (sizeof(dirbuf) * i), 0);
		}
		close(inlu);
		if(chdir("..")) {
			fprintf(stderr,"tar: fatal 'cd ..' error, abort.\n");
			endprog(3);
		}
		return;
	}
/*eject*/
/*
*	regular file handling.
*/

	tomodes(&stbuf);

	r = longname;
	for (s = dirinfo.dbuf.name, i=0; (*s++ = *r++) && i < NAMSIZ; i++);
	if (i >= NAMSIZ) {
		fprintf(stderr, "tar: %s: file name too long\n", longname);
		close(inlu);
		return;
	}
/*
*	handle link to files.
*/
	if (stbuf.st_nlink > 1) {
		struct linkbuf *lp;
		int found = 0;

		for (lp = ihead; lp != NULL; lp = lp->nextp) {
			if (lp->inum == stbuf.st_ino && lp->devnum == stbuf.st_dev) {
				found++;
				break;
			}
		}
		if (found) {
			strcpy(dirinfo.dbuf.linkname, lp->pathname);
			dirinfo.dbuf.linkflag = '1';
			sprintf(dirinfo.dbuf.chksum, "%6o", calcchksum());
			if(mulvol && tblkpos + 1 >= tblkmax)
				newvol();
			iowrtape(&dirinfo, BLKSIZ);
			if (vopt) {
				if(mulvol)
					fprintf(Logfile, "seek = %luK\t", K(tblkpos));
				fprintf(Logfile, "a %s ", longname);
				fprintf(Logfile, "link to %s\n", lp->pathname);
			}
			lp->count--;
			close(inlu);
			return;
		}
		else {
			lp = (struct linkbuf *) malloc(sizeof(*lp));
			if (lp == NULL) {
				if (freemem) {
					fprintf(stderr, "tar: Out of memory. Link information lost\n");
					freemem = 0;
				}
			}
			else {
				lp->nextp = ihead;
				ihead = lp;
				lp->inum = stbuf.st_ino;
				lp->devnum = stbuf.st_dev;
				lp->count = stbuf.st_nlink - 1;
				strcpy(lp->pathname, longname);
			}
		}
	}

	noblks = CONVBLKS(stbuf.st_size);

	/* end of volume. */
	if (mulvol && tblkpos + noblks + 1 > tblkmax) { /* file won't fit */
	    	if (tblkmax - tblkpos >= EXTMIN	/* & floppy has room */
	    	    && noblks + 1 >= tblkmax / 10) {	/* & pretty big file */
			splitfile(longname, inlu);
			return;
		}
		newvol();	/* not worth it--just get new volume */
	}
/*eject*/
/*
*	main copy out loop.
*/
	if(vopt) {
		if(mulvol)
			fprintf(Logfile, "seek = %luK\t", K(tblkpos));
		fprintf(Logfile, "a %s ", longname);
		if (mulvol)
			fprintf(Logfile, "%luK\n", K(noblks));
		else
			fprintf(Logfile, "%lu tape blocks\n", noblks);
	}
	sprintf(dirinfo.dbuf.chksum, "%6o", calcchksum());
	noblks = putext(noblks, inlu);		/* write an extent. */
	if(noblks != 0 || read(inlu, buf, BLKSIZ) != 0) {	/* size left must be zero */
		fprintf(Logfile, "tar: %s file changed size.\n", longname);
	}
	close(inlu);
	if(mopt) {
		if(stat(shortname,&sb) >= 0)	/* restat to get fresh mod time */
			stbuf.st_mtime = sb.st_mtime;
		timep[0] = stbuf.st_atime;	/* atime before backup. */
		timep[1] = stbuf.st_mtime;	/* mtime after backup. */
		utime(shortname,timep);
	}
	while(noblks-- > 0) {
		tapepadd();
	}
	return(0);
}
/*eject*/
/*
*	Split a file across volumes.
*	----------------------------
*/
splitfile(longname, inlu)
BYTE_1 *longname;				/* full filename. */
int inlu;					/* lu no for input file. */
{
	long noblks, bytes, s;
	char buf[BLKSIZ];
	register i, extents;

	noblks = CONVBLKS(stbuf.st_size);	/* noblks file needs */

	/* # extents =
	 *	size of file after using up rest of this floppy
	 *		noblks - (tblkmax - tblkpos) + 1	(for header)
	 *	plus roundup value before divide by tblkmax-1
	 *		+ (tblkmax - 1) - 1
	 *	all divided by tblkmax-1 (one block for each header).
	 * this gives
	 *	(noblks - tblkmax + tblkpos + 1 + tblkmax - 2)/(tblkmax-1)
	 * which reduces to the expression used.
	 * one is added to account for this first extent.
	 */
	extents = (noblks + tblkpos - 1L)/(tblkmax - 1L) + 1;

	if (extents < 2 || extents > MAXEXT) {	/* let's be reasonable */
		fprintf(stderr, "tar: %s needs unusual number of volumes to split\ntar: %s not dumped\n", longname, longname);
		return;
	}
	sprintf(dirinfo.dbuf.extotal, "%o", extents);	/* # extents */
	bytes = stbuf.st_size;
	sprintf(dirinfo.dbuf.efsize, "%lo", bytes);

	fprintf(stderr, "tar: large file %s needs %d extents.\ntar: current device seek position = %luK\n", longname, extents, K(tblkpos));

	s = (tblkmax - tblkpos - 1) * BLKSIZ;
	for (i = 1; i <= extents; i++) {
		if (i > 1) {
			newvol();
			if (i == extents)
				s = bytes;	/* last ext. gets true bytes */
			else
				s = (tblkmax - 1)*BLKSIZ; /* whole volume */
		}
		bytes -= s;
		noblks = CONVBLKS(s);

		sprintf(dirinfo.dbuf.size, "%lo", s);
		sprintf(dirinfo.dbuf.extno, "%o", i);
		sprintf(dirinfo.dbuf.chksum, "%6o", calcchksum());

		if (vopt)
			fprintf(Logfile, "+++ a %s %luK [extent #%d of %d]\n",
				longname, K(noblks), i, extents);
		noblks = putext(noblks, inlu);	/* write an extent. */
		if (noblks != 0) {
			fprintf(stderr, "tar: %s: file changed size\n", longname);
			fprintf(stderr, "tar: aborting split file %s\n", longname);
			close(inlu);
			return;
		}
	}
	close(inlu);
	if (vopt)
		fprintf(Logfile, "a %s %luK (in %d extents)\n",
			longname, K(CONVBLKS(stbuf.st_size)), extents);
}
/*eject*/
/*
*	Put an extent onto file.
*	------------------------
*/
putext(noblks, inlu)
BYTE_4 noblks;					/* size of this extent. */
int inlu;
{
	REG int i, j;
	REG BYTE_1 *s;

	iowrtape((BYTE_1 *) &dirinfo, BLKSIZ);	/* write file header. */
	while(noblks > 0) {			/* write until no blocks left. */
		if((i = io(F_UWRITE, tapelu, 1, 0)) <= 0) {	/* ask for bcnt */
			fprintf(stderr, "tar: tape write error.\n");
			endprog(1);
		}
		s = (BYTE_1 *) io(F_UREAD, tapelu, 0, 0);	/* ask for bad. */
		if((noblks * BLKSIZ) < i) {	/* don't read more than necessary. */
			i = noblks * BLKSIZ;
		}
		i = read(inlu, s, i);		/* read no bytes. */
		if((i == 0) || term)
			return(noblks);		/* eof. */
		else if(i < 0) {
			fprintf(stderr, "tar: read error on input file.\n");
			endprog(1);
		}
		if(i & (BLKSIZ - 1)) {		/* need to align tape? */
			i += BLKSIZ;
			i &= (-BLKSIZ);
		}
		iowrtape(s, i);			/* write onto tape. */
		j = CONVBLKS(i);		/* get the # of blocks. */
		noblks -= j;			/* reduce blocks left. */
	}
	return(noblks);				/* all blocks written. */
}
/*eject*/
/*
*	Extract named files from tape.
*	------------------------------
*
*/
tapeextr(argv)
BYTE_1 *argv[];					/* array of string pnts to */
{						/* to be extracted. */
	int i;					/* copy or skip flag. */
	int outlu;				/* output file lu. */
	BYTE_4 fszblks;				/* files size in blocks. */
	BYTE_4 fszbytes;			/* files size in bytes. */
	REG BYTE_1 **ss;			/* pointer to file name. */
	time_t timep[2];

	tapewflg = 0;
	while(readdir() > 0) {			/* loop until no more to xtract */
		i = 0;				/* assum skip file. */
		if(*argv == 0) {		/* no files spec'ed,copy all */
			i++;
		}
		if(i == 0) {
			for(ss = argv; *ss; ss++) {
				if(prefix(*ss, dirinfo.dbuf.name)) {
					i++;	/* mark copy. */
				}
			}
		}
		if(i) {				/* if copy, check interactive. */
			if(checkw('x', dirinfo.dbuf.name) == 0) {
				i = 0;		/* user want's to skip file. */
			}
		}
/*eject*/
/*
*	main extract loop.
*/
		if(i) {
			checkdir(dirinfo.dbuf.name);
			if(dirinfo.dbuf.linkflag == '1') {
				unlink(dirinfo.dbuf.name);	/* delete output. */
				i = 0;		/* make sure no copy. */
				if(link(dirinfo.dbuf.linkname, dirinfo.dbuf.name) < 0) {
					fprintf(stderr, "tar: %s: cannot link to %s.\n", dirinfo.dbuf.name, dirinfo.dbuf.linkname);
				} else {
					if (vopt)
						fprintf(Logfile, "%s linked to %s\n", dirinfo.dbuf.name, dirinfo.dbuf.linkname);
				}
			} else {
				if((outlu = creat(dirinfo.dbuf.name,
				stbuf.st_mode & MODEMASK)) < 0) {
					fprintf(stderr, "tar: %s - cannot create\n", dirinfo.dbuf.name);
					i = 0;	/* skip file. */
				}
			}
			if(i) {			/* only su can change owner. */
				if(suflag) chown(dirinfo.dbuf.name, stbuf.st_uid, stbuf.st_gid);
			}
		}
		if(i) {
			if(extno != 0) {	/* split file? */
				if (extotal < 1 || extotal > MAXEXT)
					fprintf(stderr, "tar: ignoring bad extent info for %s\n", dirinfo.dbuf.name);
				else {
					exfilsplit(outlu);
					goto fileendprog;
				}
			}
			extno = 0;		/* file not split. */
			fszblks = CONVBLKS(fszbytes = stbuf.st_size);
			if (vopt) {
				fprintf(Logfile, "x %s, %lu bytes, ", dirinfo.dbuf.name, fszbytes);
				if (mulvol)
					fprintf(Logfile, "%luK\n", K(fszblks));
				else
					fprintf(Logfile, "%lu tape blocks\n", fszblks);
			}
			exfilext(fszbytes, outlu);
			close(outlu);		/* all done on output. */
fileendprog:
			if(mopt == 0) {		/* keep time if not 'm' spec'ed */
				time(&timep[0]);
				timep[1] = stbuf.st_mtime;
				utime(dirinfo.dbuf.name, timep);
			}
			chmod(dirinfo.dbuf.name, stbuf.st_mode&MODEMASK);
		} else {
			forwfile();		/* skip over file. */
		}
	}
}
/*eject*/
/*
*	Extract a file or a extent(part file) from tape.
*	------------------------------------------------
*/
exfilext(fszbytes, outlu)
REG BYTE_4 fszbytes;				/* file size in bytes. */
int outlu;					/* lu no for output file. */
{
	BYTE_4 fszblks;				/* Files size in blocks. */
	REG BYTE_4 i;				/* Scratch var. */
	REG BYTE_1 *s;				/* Buffer address. */

	fszblks = CONVBLKS(fszbytes);		/* Get size in blocks. */
	fszblks *= BLKSIZ;			/* Now in bytes again. */
	while(fszbytes > 0) {
		if((i = io(F_UREAD, tapelu, 1, 0)) <= 0) {	/* Ask for bcnt. */
			fprintf(stderr, "tar: tape read error.\n");
			endprog(1);
		}
		s = (BYTE_1 *) io(F_UREAD, tapelu, 0, 0);	/* Ask for bad. */
		if(fszbytes <= i) {
			i = fszbytes;		/* Less than a complete block left. */
		}
		if(write(outlu, s, i) < 0) {
			fprintf(stderr, "tar: %s: HELP - extract write error\n", dirinfo.dbuf.name);
			endprog(2);
		}
		fszbytes -= i;
		fszblks -= i;
		i = iordtape(0, i);		/* Pass the data. */
	}
	if(fszblks)
		iordtape(0, fszblks);		/* Pass the last data. */
}
/*eject*/
/*
*	Extract a splitted file from tape.
*	----------------------------------
*/
union hblock savedirinfo;			/* saved dir info about file. */

exfilsplit(outlu)
int outlu;
{
	REG i, c;
	BYTE_4 noexts;				/* read extent no from file. */
	BYTE_4 totnoexts;			/* total no of extents on file. */
	BYTE_4 fszbytes;			/* extents size in bytes. */
	BYTE_4 totfszbytes;			/* total size of the file. */
	BYTE_1 name[NAMSIZ];

	strncpy(name, dirinfo.dbuf.name, NAMSIZ);
	totfszbytes = 0;
	totnoexts = 0;

	fprintf(stderr, "tar: %s split across %d volumes\n", name, extotal);
	if (extno != 1) {	/* starting in middle of file? */
		fprintf(Logfile, "tar: first extent read is not #1\nOK to read file beginning with extent #%d (y/n) ? ", extno);
		if (askuser() != 'y') {
abortx:
			forwfile();
			close(outlu);
			return;
		}
	}
	noexts = extotal;			/* total no of extents in file. */
	for (i = extno; ; ) {
		fszbytes = stbuf.st_size;
		if (vopt)
			fprintf(Logfile, "+++ x %s [extent #%d], %lu bytes, %luK\n",
				name, extno, fszbytes, K(CONVBLKS(fszbytes)));
		exfilext(fszbytes, outlu);

		totfszbytes += fszbytes;
		totnoexts++;
		if (++i > noexts)
			break;
		strncpy(&savedirinfo, &dirinfo, BLKSIZ);
getnewvol:
		newvol();
		if(readdir() <= 0) {
			fprintf(stderr, "tar: first record is null\n");
asknewvol:
			fprintf(stderr, "tar: need volume with extent #%d of %s\n", i, name);
			goto getnewvol;
		}
		if (notsame()) {
			fprintf(stderr, "tar: first file on that volume is not the same file\n");
			goto asknewvol;
		}
		if (i != extno) {
			fprintf(stderr, "tar: extent #%d received out of order\ntar: should be #%d\n", extno, i);
			fprintf(stderr, "Ignore error, Abort this file, or load New volume (i/a/n) ? ");
			c = askuser();
			if (c == 'a')
				goto abortx;
			if (c != 'i')		/* default to new volume */
				goto asknewvol;
			i = extno;		/* okay, start from there */
		}
	}
	fszbytes = stbuf.st_size;
	if (vopt)
		fprintf(Logfile, "x %s (in %d extents), %lu bytes, %luK\n",
			name, totnoexts, totfszbytes, K(CONVBLKS(totfszbytes)));
}
/*eject*/
/*
*	List contents on tape.
*	----------------------
*/
tapelist()
{
	while(readdir() > 0) {			/* loop until EOT. */
		if(vopt) longt(&stbuf);	/* long listing? */
		fprintf(Logfile, "%s", dirinfo.dbuf.name);	/* list file name. */
		if(extno != 0) {
			if (vopt)
				fprintf(Logfile, "\n [extent #%d of %d] %lu bytes total",
					extno, extotal, efsize);
			else
				fprintf(Logfile, " [extent #%d of %d]", extno, extotal);
		}
		if (dirinfo.dbuf.linkflag == '1')
			fprintf(Logfile, " linked to %s", dirinfo.dbuf.linkname);
		fprintf(Logfile, "\n");
		forwfile();
	}
}
