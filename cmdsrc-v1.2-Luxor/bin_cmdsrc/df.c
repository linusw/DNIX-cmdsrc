/*
 *	df [ filesystem/blockdevice ... ]
 *
 *	Copyright (C) DIAB T{by 1984
 *	Written by: Per-Arne Svensson 
 */

/*
 *	Changed 84-05-24:  Removed unnessesary floating point crunch.
 *									(paf)
 *	Changed printout to display k bytes.
 *	Re-wrote expression calculating diskspace in percent to avoid
 *	overflow at calculation.	84-06-05 LA
 */
/*
 *	Compute free disk blocks
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysfile.h>
#include "../cmd.h"
#include "../cmd_err.h"

#ifdef	M_BYTESWAP
#define	DISKPTR(L)	(long)(L)
#else
long	swapptr();
#define	DISKPTR(L)	(swapptr(L))
#endif
/*	External functions						*/
/*	==================						*/
int	bit_cnt();		/*	Count number of bits in a byte	*/
char	*mnt_tab();		/*	Get root of mounted device	*/
char	*dname();		/*	Get directory name of a file	*/

/*	main program							*/
/*	============							*/
main(argc, argv)
register int argc;
register char *argv[];

{
struct	stat	status;
int	result = 0;

	/*	Initiate prompt string					*/
	PRMPT = *argv;

	--argc;
	++argv;

	/*	Move to root directory					*/
	chdir(SDELIM);

	/*	If no arguments scan the entire file system		*/
	if (!*argv)
		df_all();

	while (*argv) {
		stat(*argv, &status);
		if ((status.st_mode &S_IFMT) == S_IFCHR) {
			char *f_sys;
			if ((f_sys = mnt_tab(*argv)) != NULL)
				result = df(f_sys);
			else fprintf(stderr, NOTMTD, *argv);
		} else
			result = df(*argv);
		++argv;
	}
	exit(0);
}

/*	Function for computing disk-free space - df()			*/
/*	=============================================			*/
df(filesys)
char	*filesys;
{
	register int	freecnt, chr;
	register FILE	*bitmap;
	register FILE	*vol_file;
	struct	sysfile	sysfile;	/*	Sysfile record		*/
	daddr_t	vsize;			/*	Volume size		*/
	daddr_t	bsize;			/*	Volume block size	*/
	daddr_t	bytes;			/*	Bytes rounded to blocks	*/
	int	prc_used;		/*	Percentage of vol used	*/
	char	name[MAXN];

	/*	Make the filename path to open				*/
	if ((strlen(filesys) + strlen(SYS_DIR) + strlen(BITMAP_FILE) + 3) > MAXN) {
		fprintf(stderr, LNGNAM, filesys);
		return(-1);
	}
	sprintf(name, "%s%s%s%s%s", filesys, SDELIM, SYS_DIR, SDELIM, BITMAP_FILE);

	/*	Try to open the filesystems bitmap			*/
	if ((bitmap = fopen(name, "r")) == NULL) {
		fprintf(stderr, NOOPEN, name);
		return(-1);
	}

	freecnt = 0;

	while ((chr = getc(bitmap)) != EOF)
		/*	Count the bits that are zero in the byte	*/
		freecnt += bit_cnt(chr, 0);

	/*	Close the bitmap file					*/
	fclose(bitmap);

	/*	Make the filename path to open				*/
	if ((strlen(filesys) + strlen(SYS_DIR) + strlen(VOLUME_FILE) + 3) > MAXN) {
		fprintf(stderr, LNGNAM, filesys);
		return(-1);
	}
	sprintf(name, "%s%s%s%s%s", filesys, SDELIM, SYS_DIR, SDELIM, VOLUME_FILE);

	/*	Try to open the filesystems volume descriptor file	*/
	if ((vol_file = fopen(name, "r")) == NULL) {
		fprintf(stderr, NOOPEN, name);
		return(-1);

	}

	/*	Read the volume descriptor record			*/
	if (fread(&sysfile, 1, sizeof(sysfile), vol_file) != sizeof(sysfile)) {
		fprintf(stderr, RDERR, name);
		return(-1);
	}

	/*	Close the volume descriptor file			*/
	fclose(vol_file);

	/*	Fetch the volume size					*/
	vsize = DISKPTR(sysfile.s_vlsiz);

	/*	Fetch the volume block size				*/
	bsize = DISKPTR(sysfile.s_bksiz);

	/*	Calculate the number of used bytes, rounded to blocks	*/
	bytes = freecnt * bsize;

	/*	Get the percentage of used blocks			*/
	prc_used = ((long)bytes)/( (long)vsize/100 ); /* Avoid overflow - LA */

	/*	Print the volume information				*/
printf("%s:\t%s\t - %2d%%\t : blksize %ld\t : %ld blks\t : %ldk bytes\n",
		PRMPT, filesys, prc_used, bsize, freecnt, bytes/1024);
}


/*	Function for determining mounted file system mnt_tab()		*/
/*	======================================================		*/
#include <sys/mtab.h>

char	*
mnt_tab(device)
char	*device;
{
	register FILE	*mtab_str;
	register struct	mtab mtab;
	char	*name;
	static	char	fsys[MAXN];
	*fsys = '\0';

	/*	Try to open the mount table				*/

	if ((mtab_str = fopen(name = MTAB_FILE, "r")) == NULL) {
		fprintf(stderr, NOOPEN, name);
		return(NULL);
	}

	for (name=device; *name; name++)
		if (*name == DELIM)
			device = name;
	if (*++device == '\0')
		device--;

	while (fread(&mtab, sizeof(mtab), 1, mtab_str) != NULL)
		if (strcmp(device, dname(mtab.spec)) == 0)
			strcpy(fsys, mtab.file);

	fclose(mtab_str);
	return(*fsys?fsys:NULL);
}

/*	Function for determining free space on entire system df_all()	*/
/*	=============================================================	*/

df_all()
{
	register FILE	*mtab_str;
	register struct	mtab mtab;
	char	*name;

	/*	Show free space on root file system			*/
	df(SDELIM);

	/*	Try to open the mount table				*/
	if ((mtab_str = fopen(name = MTAB_FILE, "r")) == NULL) {
		fprintf(stderr, NOOPEN, name);
		return(-1);
	}
	while (fread(&mtab, sizeof(struct mtab), 1, mtab_str) != NULL)
		df(mtab.file);
	return(0);
}

/*	Function for counting bits in a byte bit_cnt()		*/
/*	======================================================		*/
bit_cnt(byte, sign)
register int	byte;
int	sign;
{
	static	char bit_tab[256];
	static	int init = 0;
	if (!init) {
		register int i,j,k;
		for (i=0; i< sizeof(bit_tab); i++) {
			k = 0;
			j = i;
			while (j) {
				if (j & 1) k++;
				j = j >> 1;
			}
			bit_tab[i] = (char)k;
		}
		init = TRUE;
	}
	return(sign ? (int)bit_tab[byte] : 8-(int)bit_tab[byte]);
}
#ifndef	M_BYTESWAP
long swapptr(ptr)
register long ptr;
{
	register long x;
	register short i;
	for (i=0; i<4; i++) {
		x = x<<8 | (ptr&0xff);
		ptr >>= 8;
	}
	return(x);
}
#endif
