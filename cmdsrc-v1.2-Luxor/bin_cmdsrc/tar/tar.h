#define BYTE_1 char
#define BYTE_4 int
#define REG register
#define EXT extern

#define BLKSIZ	512				/* block size on tape. */
#define MXBLKNO	20				/* maximum blocksize on tape. 10k */
#define NAMSIZ	100				/* max length on a file name. */
#define MODEMASK 07777				/* file mode mask. */
#define MAXEXT	100				/* reasonable max extent no. */
#define EXTMIN	50				/* min blks left on fpy before split. */

#define	CONVBLKS(bytes)	(((bytes) + BLKSIZ - 1)/BLKSIZ)	/* useful roundup */
#define K(tnoblks)	((tnoblks+1)/2)		/* Kbytes for printing. */
/*
*	File header information block.
*/
union hblock {
	BYTE_1 dummy[BLKSIZ];
	struct header {
		BYTE_1 name[NAMSIZ];		/* files name. */
		BYTE_1 mode[8];			/* files mode. */
		BYTE_1 uid[8];			/* user id. */
		BYTE_1 gid[8];			/* group id. */
		BYTE_1 size[12];		/* size of this extent. */
		BYTE_1 mtime[12];		/* modification time. */
		BYTE_1 chksum[8];		/* checksum. */
		BYTE_1 linkflag;		/* 1 if file is linked. */
		BYTE_1 linkname[NAMSIZ];	/* file name. */
		BYTE_1 extno[4];		/* extent no. */
		BYTE_1 extotal[4];		/* total no of extents. */
		BYTE_1 efsize[12];		/* size of entire file. */
	} dbuf;
};
/*
*	Structure for keep track of links.
*/
struct linkbuf {
	ino_t	inum;				/* inode no */
	dev_t	devnum;				/* device no. */
	int	count;				/* link count. */
	BYTE_1	pathname[NAMSIZ];		/* name of link. */
	struct	linkbuf	*nextp;			/* link to next node. */
};
