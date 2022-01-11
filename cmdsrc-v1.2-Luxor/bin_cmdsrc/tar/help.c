/*
*	Help routines for 'tar'.
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <stdio.h>
#include <signal.h>
#include <mac.h>
#include <dnix/fcodes.h>
#include "tar.h"

EXT int iordtape(), iowrtape();
EXT int copt, mopt, popt, ropt, topt;
EXT int sopt, vopt, wopt, xopt, yopt;
EXT int tapelu, term, chksum, suflag;
EXT int cblkno, linkerrok, tblkpos, first, tblksz;
EXT int mulvol, NotTape, tapewflg, extno, extotal, efsize;
EXT struct stat stbuf;
EXT struct linkbuf *ihead;
EXT union hblock dirinfo, savedirinfo;
EXT BYTE_1 *tfile, tname[], *tapefile;
EXT FILE *Logfile;
daddr_t bsrch(), low, high;
/*eject*/
/*
*	Checks if the new read file is the same
*	as the old one.
*/
notsame()
{
	return(
	    strncmp(savedirinfo.dbuf.name, dirinfo.dbuf.name, NAMSIZ)
	    || strcmp(savedirinfo.dbuf.mode, dirinfo.dbuf.mode)
	    || strcmp(savedirinfo.dbuf.uid, dirinfo.dbuf.uid)
	    || strcmp(savedirinfo.dbuf.gid, dirinfo.dbuf.gid)
	    || strcmp(savedirinfo.dbuf.mtime, dirinfo.dbuf.mtime)
	    || savedirinfo.dbuf.linkflag != dirinfo.dbuf.linkflag
	    || strncmp(savedirinfo.dbuf.linkname, dirinfo.dbuf.linkname, NAMSIZ)
	    || strcmp(savedirinfo.dbuf.extotal, dirinfo.dbuf.extotal)
	    || strcmp(savedirinfo.dbuf.efsize, dirinfo.dbuf.efsize)
	);
}
/*eject*/
/*
*	readdir.
*	--------
*
*	reads directory entry and puts the dir info
*	into 'stbuf' after it's been converted from ascii.
*/
readdir()
{
	REG struct stat *sp;			/* pointer to stat structure. */
	int i;

	while(1) {
		if((i = iordtape(&dirinfo, BLKSIZ)) == 0) {	/* eom. */
			return(0);
		}
		if(i < 0) {
			io(F_SEEK, tapelu, BLKSIZ, 1);	/* step over this block. */
			continue;
		}
		if (dirinfo.dbuf.name[0] == '\0')
			return(0);		/* eot. */
		sp = &stbuf;
		sscanf(dirinfo.dbuf.mode, "%o", &i);
		sp->st_mode = i;
		sscanf(dirinfo.dbuf.uid, "%o", &i);
		sp->st_uid = i;
		sscanf(dirinfo.dbuf.gid, "%o", &i);
		sp->st_gid = i;
		sscanf(dirinfo.dbuf.size, "%lo", &sp->st_size);
		sscanf(dirinfo.dbuf.mtime, "%lo", &sp->st_mtime);
		sscanf(dirinfo.dbuf.chksum, "%o", &chksum);
		if(dirinfo.dbuf.extno[0] != '\0') {	/* file split if no zero. */
			sscanf(dirinfo.dbuf.extno, "%o", &extno);
			sscanf(dirinfo.dbuf.extotal, "%o", &extotal);
			sscanf(dirinfo.dbuf.efsize, "%lo", &efsize);
		} else
			extno = 0;		/* mark file not split. */
		if (chksum != calcchksum()) {
			if(yopt) 
				continue;	/* force scan after dir info. */
			fprintf(stderr, "tar: directory checksum error.\n");
			endprog(2);
		}
		if (tfile != NULL)
			fprintf(tfile, "%s %s\n", dirinfo.dbuf.name, dirinfo.dbuf.mtime);
		return(BLKSIZ);			/* return no of read chars. */
	}
}
/*eject*/
/*
*	Forward file.
*	-------------
*
*	skip to next file on the tape.
*
*/
forwfile()
{
	BYTE_4 noblks;

	if (dirinfo.dbuf.linkflag == '1')
		return;				/* if a link, no data follows. */
	noblks = CONVBLKS(stbuf.st_size);	/* get no of blocks to skip. */
	if(noblks) {
		if (NotTape)
			diskseek(noblks);
		else
			iordtape(0, noblks*BLKSIZ);
	}
}
/*
*	Write trailer on output volume so it can be removed.
*/
closevol()
{
	REG int i;

	tapepadd();
/*
*	always full last block to avoid
*	random positioning problem.
*/
	i = io(F_UWRITE, tapelu, 1, 0);		/* get bytes left. */
	i = CONVBLKS(i);			/* convert to blocks. */
	for(; i > 0; i--) {
		tapepadd();			/* make sure full last block. */
	}
}
/*
*	Fill an empty block with zeros.
*/
tapepadd()
{
	BYTE_1 buf[BLKSIZ];
	BYTE_1 *s;

	for(s = buf; s < &buf[BLKSIZ]; *s++ = 0);
	iowrtape(buf, BLKSIZ);
}
/*eject*/
/*
*	Dump file info on output.
*	file size, time, user id and group id.
*/
longt(st)
REG struct stat *st;
{
	REG BYTE_1 *s, *r;
	BYTE_1 buf[100];
	BYTE_1 *ctime(), *prmode();

	s = prmode(st, buf);
	sprintf(s, "%3d/%-3d", st->st_uid, st->st_gid);
	s += 7;
	sprintf(s, "%7lu", st->st_size);
	s += 7;
	r = ctime(&st->st_mtime);
	sprintf(s, " %-12.12s %-4.4s ", r+4, r+20);
	fprintf(Logfile, "%s", buf);
}
/*eject*/
/*
*	Create a string with access modes.
*	----------------------------------
*/
#define	SUID	04000
#define	SGID	02000
#define	ROWN	0400
#define	WOWN	0200
#define	XOWN	0100
#define	RGRP	040
#define	WGRP	020
#define	XGRP	010
#define	ROTH	04
#define	WOTH	02
#define	XOTH	01
#define	STXT	01000
int	m1[] = { 1, ROWN, 'r', '-' };
int	m2[] = { 1, WOWN, 'w', '-' };
int	m3[] = { 2, SUID, 's', XOWN, 'x', '-' };
int	m4[] = { 1, RGRP, 'r', '-' };
int	m5[] = { 1, WGRP, 'w', '-' };
int	m6[] = { 2, SGID, 's', XGRP, 'x', '-' };
int	m7[] = { 1, ROTH, 'r', '-' };
int	m8[] = { 1, WOTH, 'w', '-' };
int	m9[] = { 2, STXT, 't', XOTH, 'x', '-' };

int	*m[] = { m1, m2, m3, m4, m5, m6, m7, m8, m9};

BYTE_1 *
prmode(st, s)
REG struct stat *st;
REG BYTE_1 *s;
{
	REG int **mp, *ip, i;

	for(mp = m; mp < &m[9];) {
		ip = *mp++;
		i = *ip++;			/* get counter. */
		for(; i > 0; i--) {
			if(st->st_mode & *ip++)
				break;
			else
				ip++;
		}
		*s++ = *ip;
	}
	return(s);
}
/*eject*/
/*
*	Routine to check directory.
*/
checkdir(name)
REG BYTE_1 *name;
{
	REG BYTE_1 *s;
	int i;
	for (s = name; *s; s++) {
		if (*s == '/') {
			*s = '\0';
			if(access(name, 01) < 0) {	/* directory present? */
				if(fork() == 0) {	/* create it else. */
					execl("/bin/mkdir", "mkdir", name, 0);
					execl("/usr/bin/mkdir", "mkdir", name, 0);
					fprintf(stderr, "tar: cannot find mkdir!\n");
					endprog(0);
				}
				while (wait(&i) >= 0);
						/* change owner if su. */
				if(suflag) chown(name, stbuf.st_uid, stbuf.st_gid);
			}
			*s = '/';
		}
	}
}
/*eject*/
/*
*	Interupt routines.
*	------------------
*/
onintr()
{
	signal(SIGINT, SIG_IGN);
	term++;
}

onquit()
{
	signal(SIGQUIT, SIG_IGN);
	term++;
}

onhup()
{
	signal(SIGHUP, SIG_IGN);
	term++;
}

/*	currently not implemented.
onterm()
{
	signal(SIGTERM, SIG_IGN);
	term++;
}
*/
/*eject*/
/*
*	Convert modes from binary to ascii strings.
*	-------------------------------------------
*/
tomodes(sp)
REG struct stat *sp;				/* pointer to a stat struc. */
{
	REG BYTE_1 *s;				/* buffer pointer. */

	for(s = dirinfo.dummy; s < &dirinfo.dummy[BLKSIZ]; s++)
		*s = 0;				/* clear buffer first. */
	sprintf(dirinfo.dbuf.mode, "%6o ", sp->st_mode & MODEMASK);
	sprintf(dirinfo.dbuf.uid, "%6o ", sp->st_uid);
	sprintf(dirinfo.dbuf.gid, "%6o ", sp->st_gid);
	sprintf(dirinfo.dbuf.size, "%11lo ", sp->st_size);
	sprintf(dirinfo.dbuf.mtime, "%11lo ", sp->st_mtime);
}
/*
*	Calculate checksum.
*/
calcchksum()
{
	REG i;
	REG BYTE_1 *s;

	for (s = dirinfo.dbuf.chksum; s < &dirinfo.dbuf.chksum[sizeof(dirinfo.dbuf.chksum)]; s++)
		*s = ' ';
	i = 0;
	for (s = dirinfo.dummy; s < &dirinfo.dummy[BLKSIZ]; s++)
		i += *s;
	return(i);
}
/*eject*/
/*
*	If in interactive mode, ask user what to do.
*/
checkw(c, name)
BYTE_1 *name;
{
	if (wopt) {
		fprintf(Logfile, "%c ", c);
		if (vopt)
			longt(&stbuf);
		fprintf(Logfile, "%s: ", name);
		if (askuser() == 'y'){
			return(1);
		}
		return(0);
	}
	return(1);
}

askuser()
{
	REG int c;

	c = getchar();
	if (c != '\n')
		while (getchar() != '\n');
	else c = 'n';
	return((c >= 'A' && c <= 'Z') ? c + ('a'-'A') : c);
}
/*eject*/
/*
*	Check file time in temp file and see if file modified.
*/
checkupdate(arg)
BYTE_1 *arg;
{
	BYTE_1 name[100];
	BYTE_4 mtime;
	daddr_t seekp;
	daddr_t	lookup();

	rewind(tfile);
	for (;;) {
		if ((seekp = lookup(arg)) < 0)
			return(1);
		fseek(tfile, seekp, 0);
		fscanf(tfile, "%s %lo", name, &mtime);
		if (stbuf.st_mtime > mtime)
			return(1);
		else
			return(0);
	}
}
/*eject*/
/*
*	Tell user start conditions.
*	---------------------------
*/
telldef()
{
	fprintf(stderr, "tar usage: tar -{txru}[cvfbklmn] [tapefile] [noblksize] [tapesize] files ...\n");
	endprog(1);
}
/*eject*/
/*
*	Mount new output or input volume.
*/
newvol()
{
	REG int c;
	REG int fc, lu;

	if (tapewflg) {
		closevol();			/* padd out last records. */
		sync();				/* make sure sync. */
		tblkpos = 0;
	} else
		first = 0;
	if(io(F_CLOSE, tapelu, 0, 0) < 0) {
		fprintf(stderr, "tar: write error at closee.\n");
		endprog(3);
	}
	fprintf(stderr, "tar: please mount new volume.");
	fseek(stdin, 0L, 2);			/* in case stdin, pos to last. */
	while ((c = getchar()) != '\n' && ! term)
		if (c == EOF)
			endprog(0);
	if (term)
		endprog(0);
	fc = F_OPEN;
	lu = 0;					/* read only attr and stdin. */
	if(tapewflg) {
		lu = 2;				/* read,write attr. */
	}
	if(strcmp(tapefile, "-") == 0) {
		fc = F_DLU;
		if(tapewflg) {
			lu = 1;			/* let stdout be out lu. */
		}
	}
	tapelu = io(fc, lu, tapefile, tblksz*BLKSIZ);
	if (tapelu < 0) {
		fprintf(stderr, "tar: cannot reopen %s (%s)\n", tapewflg ? "output" : "input", tapefile);
		endprog(2);
	}
}
/*eject*/
/*
*	Terminate session and close files.
*/
endprog(n)
{
	if(tapelu >= 0 && io(F_CLOSE, tapelu, 0, 0) < 0)
		fprintf(stderr, "tar: write error at close.\n");
	unlink(tname);
	exit(n);
}
/*
*	Check matching file name.
*
*	gives match on complete name and if 's2' ends
*	in '/' which means a complete directory is searched.
*/
prefix(s1, s2)
REG BYTE_1 *s1, *s2;
{
	REG BYTE_1 *r1, *r2;

	while((*s1 == '.') && (*(s1+1) == '/'))
		s1 += 2;
	while((*s2 == '.') && (*(s2+1) == '/'))
		s2 += 2;
	r2 = s2;
	r1 = s1;
	while (*s1)
		if (*s1++ != *s2++)
			return(wmatch(r2, r1, TRUE) ? 1 : 0);
/*
*	Complete match.
*/
	if(*s2)
		return(*s2 == '/');		/* only directory? */
	return(1);				/* file name ok. */
}
/*
*	Get working directory name.
*/
getwd(s)
BYTE_1 *s;
{
	int i;
	int	pipdes[2];

	pipe(pipdes);
	if ((i = fork()) == 0) {
		close(1);
		dup(pipdes[1]);
		execl("/bin/pwd", "pwd", 0);
		execl("/usr/bin/pwd", "pwd", 0);
		fprintf(stderr, "tar: pwd failed!\n");
		fprintf(stderr, "/\n");
		exit(1);
	}
	if (i == -1) {
		fprintf(stderr, "tar: No process to get directory name!\n");
		endprog(7);
	}
	while (wait((int *)NULL) != i);
	read(pipdes[0], s, 50);
	while(*s != '\n')
		s++;
	*s = '\0';
	close(pipdes[0]);
	close(pipdes[1]);
}
/*eject*/
/*
*	copy n characters.
*/
strncpy(d, s, i)
REG BYTE_1 *s, *d;
REG int i;
{
	for(; i > 0; i--) {
		*d++ = *s++;
	}
}
/*
*	Routines for searching in tempory file.
*
*	look for modified time etc.
*/
#define	N	200
int	njab;
daddr_t
lookup(s)
BYTE_1 *s;
{
	REG i;
	daddr_t a;

	for(i=0; s[i]; i++)
		if(s[i] == ' ')
			break;
	a = bsrch(s, i, low, high);
	return(a);
}

daddr_t
bsrch(s, n, l, h)
daddr_t l, h;
BYTE_1 *s;
{
	REG i, j;
	BYTE_1 b[N];
	daddr_t m, m1;

	njab = 0;

loop:
	if(l >= h)
		return(-1L);
	m = l + (h-l)/2 - N/2;
	if(m < l)
		m = l;
	fseek(tfile, m, 0);
	fread(b, 1, N, tfile);
	njab++;
	for(i=0; i<N; i++) {
		if(b[i] == '\n')
			break;
		m++;
	}
	if(m >= h)
		return(-1L);
	m1 = m;
	j = i;
	for(i++; i<N; i++) {
		m1++;
		if(b[i] == '\n')
			break;
	}
	i = cmp(b+j, s, n);
	if(i < 0) {
		h = m;
		goto loop;
	}
	if(i > 0) {
		l = m1;
		goto loop;
	}
	return(m);
}

cmp(b, s, n)
REG BYTE_1 *b, *s;
{
	REG i;

	if(b[0] != '\n')
		exit(2);
	for(i=0; i<n; i++) {
		if(b[i+1] > s[i])
			return(-1);
		if(b[i+1] < s[i])
			return(1);
	}
	return(b[i+1] == ' '? 0 : -1);
}
/*eject*/
/*
*	Seek on disk to correct position.
*	---------------------------------
*/
diskseek(noblks)
long noblks;
{
	fprintf(stderr, "tar: seek not implemented yet.\n");
	endprog(3);
}

/*eject*/
/*
*	Write requested number of bytes onto tape.
*	------------------------------------------
*/
iowrtape(s, sz)
BYTE_1 *s;					/* buffer address. */
int sz;						/* byte count. */
{
	int fc;

	fc = F_WRITE;
	if(sopt) {				/* turbo version? */
		fc |= F_NOWAIT;
	}
	if(io(fc, tapelu, s, sz) != sz) {	/* write data. */
		fprintf(stderr, "tar: tape write error.\n");
		endprog(2);
	}
	tblkpos += CONVBLKS(sz);		/* step tape position. */
	return(sz);
}
/*eject*/
/*
*	Read requested number of bytes from tape.
*	-----------------------------------------
*/
iordtape(s, sz)
REG BYTE_1 *s;					/* buffer address. */
REG int sz;					/* buffers max size. */
{
	REG int i, fc;
	REG BYTE_1 *r;

	fc = F_READ;
	if(sopt) {				/* turbo version? */
		fc |= F_NOWAIT;
	}
	r = s;
	do {
		if((i = io(fc, tapelu, s, sz)) < 0) {	/* read data. */
iordtap1:
			fprintf(stderr, "tar: tape read error.\n");
			if(yopt && (ropt == 0)) {	/* not replace and magic 'y'. */
				return(i);
			}
			endprog(3);
		}
		s += i;
		sz -= i;
	} while(i > 0 && sz > 0);
	sz = s - r;
	if(sz == 0) {
		fprintf(stderr,"tar: premature EOFf.\n");
		endprog(3);
	}
	if(first == 0) {
		first++;
		if((i = io(F_UREAD, tapelu, 2, 0)) < 0) {
			goto iordtap1;
		}
		fprintf(stderr, "tar: blockfactor = %d\n", i  / BLKSIZ);
		tblksz = i / BLKSIZ;		/* set up max block no. */
		if(tblksz == 0) {
			fprintf(stderr, "tar: Internal error, blksz = 0.\n");
			endprog(2);
		}
	}
	return(sz);
}
/*eject*/

/*-----------------------------------------------------------------*/
/*  This routine tries to match 'wild', a string that may contain  */
/*  wildcard characters ( *, ?, or [...] ), against the string     */
/*  'name', a filename or a case variable.                         */
/*-----------------------------------------------------------------*/
#define WC_CHAR	'?'
#define WC_STR	'*'
#define WC_LPAR	'['
#define WC_RPAR	']'
#define WC_SEQ	'-'
#define C_WDIR	'.'
#define C_NUL	'\0'
#define UNQUOTE	127

wmatch(name,wild,firstch)
register char *name,*wild;
register int firstch;
BEGIN
    register char nc,wc;
    
    nc = *name++; wc= *wild++;
    IF firstch ANDF nc == C_WDIR ANDF wc != nc THEN
	return(FALSE);
   FI
    IF wc == WC_LPAR THEN
        BEGIN
            int found = FALSE;
            char lc = (char)127;

            WHILE wc = *wild++ DO
                IF wc == WC_RPAR THEN
                    return(found ? wmatch(name,wild,FALSE) : FALSE);
                ELIF wc == WC_SEQ THEN
                    IF lc <= nc ANDF nc <= (*wild++) THEN found = TRUE FI
                ELSE
                    IF nc == (lc = wc) THEN found = TRUE FI
                FI
            OD
            return(FALSE);
        END
    ELIF wc == WC_CHAR THEN			/*  '?'  */
        return( nc != C_WDIR ANDF nc ? wmatch(name,wild,FALSE) : FALSE);
    ELIF wc == WC_STR THEN			/*  '*'  */
        IF *wild == C_NUL THEN return(TRUE);
        ELSE --name;
            WHILE *name DO
                IF wmatch(name++,wild,FALSE) THEN return(TRUE) FI
            OD
            return(FALSE);
        FI
    ELIF wc == C_NUL THEN            
        return(nc==C_NUL);
    ELSE
    return( (nc & UNQUOTE) == (wc & UNQUOTE) ? wmatch(name,wild,FALSE) : FALSE);
    FI
END
