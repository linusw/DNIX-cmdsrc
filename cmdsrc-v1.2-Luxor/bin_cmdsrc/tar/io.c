#include <dnix/errno.h>
#include <dnix/fcodes.h>
#include <dnix/itrq.h>
#include <stdio.h>
#define BYTE_4 long
#define BYTE_1 char
#define REG register
/*
*	io package.
*
*	used for speeding up the usual io routines,
*	such as write, which uses double buffered
*	no wait function.
*/
#define MX_BSZ	1024				/* current buffer size. */
#define MX_LU	4				/* max no of files to handle */
#define LS_REQ	0x01				/* outstanding request. */
#define	LS_RND	0x02				/* random address present. */
#define LS_OPEN	0x04				/* lu is opened. */
#define LS_WRIT	0x08				/* buffer is updated. */
#define LS_FIRS	0x10				/* first time read. */

struct str_lu {
	int	l_fc;				/* last used function on buf. */
	int	l_lu;				/* true dnix lu no; */
	int	l_bsz;				/* buffers max size. */
	int	l_bcnt;				/* no of bytes in buffer. */
	int	l_inx;				/* current position in buf */
	int	l_stat;				/* status flags. */
	int	l_trapq;			/* trap q lu. */
	BYTE_1	*l_bad1;			/* pointer to free buffer. */
	BYTE_1	*l_bad2;			/* pointer to buffer in access */
	BYTE_4	l_crnd;				/* current random address. */
	BYTE_4	l_qrnd;				/* requested random address. */
	BYTE_4	l_ernd;				/* end of file random address. */
};
struct str_lu lutab[MX_LU];			/* actual lu table. */
/*eject*/
/*
*
*/
io(fc, lu, pnt, sz)
int fc;						/* function code. */
int lu;						/* lu no. */
BYTE_1 *pnt;					/* buffer or fd pointer. */
int sz;						/* no bytes to read write. */
{
	int i;
	BYTE_4 crnd;				/* save rnd adr in case of err. */
	REG struct str_lu *lupnt;

	if((fc == F_OPEN) || (fc == F_CREATE) || (fc == F_DLU)) {
		return(ioopcre(fc, lu, pnt, sz));
	}
	for(lupnt = &lutab[0]; lupnt < &lutab[MX_LU]; lupnt++) {
		if(lupnt->l_lu == lu) break;
	}
	if((lupnt >= &lutab[MX_LU]) || ((lupnt->l_stat & LS_OPEN) == 0)) {
		if(lu > 2) {			/* not stdin,stdout or stderr. */
			return(EBADLU);		/* return if not opened. */
		}				/* else try open standard. */
		if((i = ioopcre(fc, lu, 0, MX_BSZ)) < 0) {
			return(i);
		}
		for(lupnt = lutab; lupnt < &lutab[MX_LU]; lupnt++) {
			if(lupnt->l_lu == lu) break;
		}
	}
	switch(fc) {
		case F_READ+F_NOWAIT:
		case F_WRITE+F_NOWAIT:
		case F_READ:
		case F_WRITE:
			crnd = lupnt->l_crnd;
			i = iordwr(fc, lupnt, pnt, sz);
			if(i >= 0) {
				if(lupnt->l_qrnd > lupnt->l_ernd) {
					lupnt->l_ernd = lupnt->l_qrnd;
				}
			} else {
				lupnt->l_qrnd = crnd;
				lupnt->l_stat &= ~LS_WRIT;
				lupnt->l_stat |= LS_RND;
			}
			break;
		case F_CLOSE:
			i = ioclose(lupnt);
			break;
		case F_SEEK:
			i = ioseek(lupnt, pnt, sz);
			break;
		case F_UREAD:
		case F_UWRITE:
			i = iourdwr(fc, lupnt, pnt, sz);
			break;
		default:
			i = EBADFC;
	}
	return(i);
}
/*eject*/
/*
*	seek function.
*/
ioseek(lupnt, pnt, sz)
REG struct str_lu *lupnt;
BYTE_1 *pnt;
int sz;
{
	BYTE_4 rnd;

	if(sz > 1) {
		return(EBADFC);
	}
	rnd = (BYTE_4) pnt;
	if(sz == 1) {				/* relative positioning. */
		rnd = rnd + lupnt->l_qrnd;
	}
	lupnt->l_qrnd = rnd;			/* set new requested rnd adr. */
	lupnt->l_stat |= LS_RND;		/* new random address present.*/
	return(rnd);
}
/*
*
*/
iodoseek(fc, lupnt)
int fc;
REG struct str_lu *lupnt;
{
	BYTE_4 rnd;
	REG int i, j;
	struct itrq trapqinfo;

	fc &= ~F_NOWAIT;
	if(lupnt->l_stat & LS_REQ) {		/* pending request? */
		svc(F_READ, lupnt->l_trapq, &trapqinfo, sizeof(struct itrq),
			0, 0, 0, 0);
		lupnt->l_stat &= ~LS_REQ;
		i = trapqinfo.it_rpar;
		if(i >= 0) {
			if((i = svc(F_SEEK, lupnt->l_lu, -lupnt->l_bsz, 1)) < 0) {
				return(i);
			}
		} else {
			return(i);
		}
	}
	j = lupnt->l_qrnd - lupnt->l_crnd;
/*
*	if record already in memory, just back
*	random position if write.
*/
	if((j >= 0) && (j < lupnt->l_bsz)) {
		i = lupnt->l_crnd;
		if(fc == F_WRITE) {
			i = svc(F_SEEK, lupnt->l_lu, lupnt->l_crnd, 0);
		}
		if(i >= 0) {
			lupnt->l_stat &= ~LS_RND;
		}
		return(i);
	}
/*
*	eventual write of current record
*	and read requested one.
*/
	if(lupnt->l_stat & LS_WRIT) {
		printf("tar: iopackage seek error.\n");
		return(EBADFC);
	}
	lupnt->l_crnd = (lupnt->l_qrnd / lupnt->l_bsz) * lupnt->l_bsz;
	lupnt->l_stat &= ~LS_RND;
	return(svc(F_SEEK, lupnt->l_lu, lupnt->l_crnd, 0));
}
/*eject*/
/*
*	handle read/write function.
*/
iordwr(fc, lupnt, pnt, sz)
int fc;
REG struct str_lu *lupnt;
REG BYTE_1 *pnt;
int sz;
{
	REG BYTE_1 *d;
	REG int j = 0;
	REG int i, fc1;

	if((lupnt->l_stat & LS_RND) && ((i = iodoseek(fc, lupnt)) < 0)) {
		return(i);
	}
	fc1 = fc & ~F_NOWAIT;			/* remove any nowait. */
	while(sz > 0) {				/* rd/wr as long we got data. */
		if((i = ioseekblk(fc, lupnt)) <= 0) {
			if(i)
				return(i);	/* Error exit */
			return(j);		/* Deliver so much as we got. */
		}
		i = lupnt->l_bcnt - lupnt->l_inx;	/* no bytes left in buffer. */
		if(fc1 == F_WRITE) {
			i = lupnt->l_bsz - lupnt->l_inx;
		}
		if(sz < i) {
			i = sz;
		}
		d = lupnt->l_bad1 + lupnt->l_inx;
		lupnt->l_inx += i;
		lupnt->l_qrnd += i;
		j += i;				/* inc total counter. */
		sz -= i;
		if(pnt) {			/* if no pnt, just pass data. */
			if(fc1 == F_WRITE) {
				lupnt->l_stat |= LS_WRIT;
				for(i; i > 0; i--) *d++ = *pnt++;	/* copy chars. */
			} else {
				for(i; i > 0; i--) *pnt++ = *d++;
			}
		}
		if(lupnt->l_inx < lupnt->l_bcnt) {
			return(j);		/* all done. */
		}
		lupnt->l_bcnt = lupnt->l_inx;
	}
	return(j);				/* return no xferred bytes. */
}
/*eject*/
/*
*	seek block routine.
*	-------------------
*/
ioseekblk(fc, lupnt)
int fc;
REG struct str_lu *lupnt;
{
	int i, fc1;
	BYTE_4 j;

	j = lupnt->l_qrnd - lupnt->l_crnd;
	i = lupnt->l_bsz;
	fc1 = fc & ~F_NOWAIT;
	if(lupnt->l_stat & LS_FIRS) {
		if(fc1 == F_READ) {
			goto firstread;
		}
	}
	if(fc1 == F_READ)
		i = lupnt->l_bcnt;
	if((j >= i) || (j < 0)) {
		if(lupnt->l_stat & LS_WRIT) {
			if((i = iosvc(F_NOWAIT+F_WRITE, lupnt, lupnt->l_bcnt)) != lupnt->l_bcnt) {
				return(EIO);
			}
			lupnt->l_crnd += lupnt->l_bcnt;
			lupnt->l_stat &= ~LS_WRIT;
			lupnt->l_bcnt = 0;	/* no bytes in buffer. */
		}
/*
*	need preread? not if after last eof.
*/
		if((fc1 == F_READ) || ((lupnt->l_crnd + lupnt->l_inx) < lupnt->l_ernd)) {
			lupnt->l_crnd += lupnt->l_bcnt;
firstread:
			if((i = iosvc(fc, lupnt, lupnt->l_bsz)) <= 0) {
				return(i);
			}
			lupnt->l_bcnt = i;
			if(lupnt->l_stat & LS_FIRS) {
				lupnt->l_bsz = i;	/* change buffer size. */
			}
		}
	}
	lupnt->l_fc = fc;
	lupnt->l_inx = lupnt->l_qrnd - lupnt->l_crnd;
	lupnt->l_stat &= ~(LS_RND+LS_FIRS);
	return(i);
}
/*eject*/
/*
*	perform DNIX system call. 
*/
iosvc(fc, lupnt, sz)
REG int fc;					/* read write function. */
REG struct str_lu *lupnt;
REG int sz;
{
	REG int i = 0;
	struct itrq trapqinfo;
	REG BYTE_1 *d;

	i = sz;					/* in case first write. */
	if(fc & F_NOWAIT) {			/* nowait handling. */
		if(((lupnt->l_stat & LS_REQ) == 0) && (fc == (F_NOWAIT+F_READ))) {
			svc(fc, lupnt->l_lu, lupnt->l_bad2, sz,
				0, 0, lupnt->l_trapq, 0);
			lupnt->l_stat |= LS_REQ;
		}
		if(lupnt->l_stat & LS_REQ) {		/* any outstanding req? */
			lupnt->l_stat &= ~LS_REQ;
			svc(F_READ, lupnt->l_trapq, &trapqinfo, sizeof(struct itrq),
				0, 0, 0, 0);
			i = trapqinfo.it_rpar;
		}
		if(i < 0) {
			lupnt->l_crnd = svc(F_SEEK, lupnt->l_lu, 0L, 1);
			return(i);
		}
		lupnt->l_stat |= LS_REQ;	/* mark outstanding req. */
		svc(fc, lupnt->l_lu, lupnt->l_bad1, sz, 0, 0, lupnt->l_trapq, 0);
swapbufad:
		d = lupnt->l_bad1;		/* swap buffer pointers. */
		lupnt->l_bad1 = lupnt->l_bad2;
		lupnt->l_bad2 = d;
		lupnt->l_stat &= ~LS_WRIT;	/* buffer written. */
	} else {
		if(lupnt->l_stat & LS_REQ) {	/* let previous req term. */
			lupnt->l_stat & ~LS_REQ;
			svc(F_READ, lupnt->l_trapq, &trapqinfo,
			sizeof(struct itrq), 0, 0, 0, 0);
			i = trapqinfo.it_rpar;	/* ret if err, or just read. */
			if(i <= 0) {		/* ret if err or eof. */
				return(i);
			}
			if(fc == F_READ) {
				goto swapbufad;	/* else just swap pointers. */
			}
		}
		i = svc(fc, lupnt->l_lu, lupnt->l_bad1,
			sz, 0, 0, 0, 0);
		lupnt->l_stat &= ~LS_WRIT;
	}
			
	return(i);
}
/*eject*/
/*
*	handle special functions.
*	-------------------------
*
*	user wants to know current buffer pointer
*	and number of bytes left in buffer.
*/
iourdwr(fc, lupnt, s, i)
int fc;						/* function code. */
REG struct str_lu *lupnt;			/* lu pointer. */
REG BYTE_1 *s;					/* buffer pointer. */
REG int i;					/* byte count. */
{
	if(i == 0) {
		i = EBADFC;
		if((int) s == 1) {		/* asking for bytes left. */
			i = lupnt->l_bcnt - lupnt->l_inx;
			if(fc == F_UWRITE) {
				i = lupnt->l_bsz - lupnt->l_inx;
			}
			while(i <= 0) {
				if((i = ioseekblk(lupnt->l_fc, lupnt)) < 0) {
					return(i);
				}
			}
		} else if((int) s == 0) {	/* asking for bad. */
			i = (int) (lupnt->l_bad1 + lupnt->l_inx);
		} else if((int) s == 2) {
			i = lupnt->l_bsz;	/* asking for buffer size. */
		}
	} else {
		i = EBADFC;
	}
	return(i);
}
/*eject*/
/*
*	handle open.
*	------------
*/
ioopcre(fc, lu, pnt, sz)
int fc;
int lu;						/* access modes. */
REG BYTE_1 *pnt;
int sz;
{
	int i;
	REG BYTE_1 *s;
	REG struct str_lu *lupnt;

	for(lupnt = lutab; lupnt < &lutab[MX_LU]; lupnt++) {
		if((lupnt->l_stat & LS_OPEN) == 0) 
			break;
	}
	if(lupnt >= &lutab[MX_LU]) {
		return(EBADLU);
	}
	if(lupnt->l_bad1 == 0) {		/* any buffer allocated. */
		if((lupnt->l_bad1 = (BYTE_1 *) calloc(sz, 1)) == 0) {
			return(EIO);		/* end of system mem. */
		}
		if((lupnt->l_bad2 = (BYTE_1 *) calloc(sz, 1)) == 0) {
			return(EIO);
		}
		lupnt->l_bsz = sz;
		lupnt->l_bcnt = 0;
	}
	if(lupnt->l_trapq <= 0) {		/* open a trapq. */
		if((lupnt->l_trapq = svc(F_OTQ, 0, 0, 0)) < 0) {
			return(EIO);
		}
	}
	if(fc == F_DLU) {
		if((lu = svc(F_DLU, lu, 0, 0)) < 0) {
			return(lu);
		}
	} else if(pnt) {			/* skip open if std open. */
		i = CDIRLU;
		while(*pnt == ' ') pnt++;	/* skip spaces. */
		while(*pnt == '/') {
			i = RDIRLU;
			pnt++;
		}
		for(s = pnt; *s; s++);
		if((lu = svc(fc, i, pnt, s - pnt, lu, 0, 0, 0)) < 0) {
			return(lu);
		}
	}
	lupnt->l_stat = LS_OPEN+LS_FIRS;
	lupnt->l_inx = 0;
	lupnt->l_ernd = 0;
	lupnt->l_crnd = 0;
	lupnt->l_qrnd = 0;
	return(lupnt->l_lu = lu);
}
/*eject*/
/*
*	handle close function.
*	----------------------
*/
ioclose(lupnt)
REG struct str_lu *lupnt;
{
	REG int i;

	if(lupnt->l_stat & LS_WRIT) {
		lupnt->l_stat &= ~LS_WRIT;
		if(lupnt->l_inx) {
			if((i = iosvc(F_WRITE, lupnt, lupnt->l_inx)) < 0) 
				return(i);
		}
	}
	lupnt->l_stat &= ~LS_OPEN;		/* mark closed. */
	svc(F_CLOSE, lupnt->l_lu, 0, 0);
	return(0);
}
/*eject*/
