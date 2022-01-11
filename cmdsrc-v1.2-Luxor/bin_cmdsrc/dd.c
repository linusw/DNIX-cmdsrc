#include <stdio.h>
#include <signal.h>
/*
 *	Disk Dump - convert and copy
 *
 *	Syntax: dd par1 par2 par3 ...
 *
 *	if=fname	input file, default standard input
 *	of=fname	output file, default standard output
 *	ibs= xxx	input block size, default 512
 *	obs= xxx	output block size, default 512
 *	bs = xxx	input and output block size
 *	cbs= xxx	conversion buffer size at ascii and ebcdic conv.
 *	skip=xxx	skip xxx input records before start copying
 *	seek=xxx	seek to record xxx on output before start copying
 *	count=xxx	copy xxx input records
 *	files=xxx	copy xxx files from input (tape)
 *	conv = par1,par2,par3,...
 *		ascii - convert ebcdic to ascii
 *		ebcdic- convert ascii to ebcdic
 *		ibm   - convert ascii to "ibm-ebcdic"    
 *		lcase - convert all upper case to lower case
 *		ucase - convert all lower case to upper case
 *		swab  - swap every pair of bytes 
 *		noerror-do not stop if error is encountered
 *		sync  - pad every input record to input block size
 *
 *	xxx - number ending with w = mult with 2
 *				 b = mult with 512
 *				 k = mult with 1024
 *	Muliplication is allowed using operator 'x' or '*'
 *
 */
 /*eject*/
char *infile=NULL;
char *outfile=NULL;
char *inbuff;
char *outbuff;
char *obp;
int ibs = 512;
int obs = 512;
int bs = 0;
int cbs = 0;
int skip = 0;
int seek = 0;
int count = 0;
int files = 0;
unsigned int mode;
int cbcnt,ibcnt,obcnt;
int spacecnt;
#define ASCII 1
#define EBCDIC	2
#define IBM	4
#define LCASE	8
#define UCASE	16
#define SWAB	32
#define NOERROR	64
#define SYNC	128

#define MAXNUMBER 	327670
char *cmd;
int firec,pirec,forec,porec,trec;
int outfd,infd;
char *sbrk();

/*eject*/
char	ebctoasc[] = {
	0000,0001,0002,0003,0234,0011,0206,0177,
	0227,0215,0216,0013,0014,0015,0016,0017,
	0020,0021,0022,0023,0235,0205,0010,0207,
	0030,0031,0222,0217,0034,0035,0036,0037,
	0200,0201,0202,0203,0204,0012,0027,0033,
	0210,0211,0212,0213,0214,0005,0006,0007,
	0220,0221,0026,0223,0224,0225,0226,0004,
	0230,0231,0232,0233,0024,0025,0236,0032,
	0040,0240,0241,0242,0243,0244,0245,0246,
	0247,0250,0133,0056,0074,0050,0053,0041,
	0046,0251,0252,0253,0254,0255,0256,0257,
	0260,0261,0135,0044,0052,0051,0073,0136,
	0055,0057,0262,0263,0264,0265,0266,0267,
	0270,0271,0174,0054,0045,0137,0076,0077,
	0272,0273,0274,0275,0276,0277,0300,0301,
	0302,0140,0072,0043,0100,0047,0075,0042,
	0303,0141,0142,0143,0144,0145,0146,0147,
	0150,0151,0304,0305,0306,0307,0310,0311,
	0312,0152,0153,0154,0155,0156,0157,0160,
	0161,0162,0313,0314,0315,0316,0317,0320,
	0321,0176,0163,0164,0165,0166,0167,0170,
	0171,0172,0322,0323,0324,0325,0326,0327,
	0330,0331,0332,0333,0334,0335,0336,0337,
	0340,0341,0342,0343,0344,0345,0346,0347,
	0173,0101,0102,0103,0104,0105,0106,0107,
	0110,0111,0350,0351,0352,0353,0354,0355,
	0175,0112,0113,0114,0115,0116,0117,0120,
	0121,0122,0356,0357,0360,0361,0362,0363,
	0134,0237,0123,0124,0125,0126,0127,0130,
	0131,0132,0364,0365,0366,0367,0370,0371,
	0060,0061,0062,0063,0064,0065,0066,0067,
	0070,0071,0372,0373,0374,0375,0376,0377,
};
/*eject*/
char	asctoebc[] = {
	0000,0001,0002,0003,0067,0055,0056,0057,
	0026,0005,0045,0013,0014,0015,0016,0017,
	0020,0021,0022,0023,0074,0075,0062,0046,
	0030,0031,0077,0047,0034,0035,0036,0037,
	0100,0117,0177,0173,0133,0154,0120,0175,
	0115,0135,0134,0116,0153,0140,0113,0141,
	0360,0361,0362,0363,0364,0365,0366,0367,
	0370,0371,0172,0136,0114,0176,0156,0157,
	0174,0301,0302,0303,0304,0305,0306,0307,
	0310,0311,0321,0322,0323,0324,0325,0326,
	0327,0330,0331,0342,0343,0344,0345,0346,
	0347,0350,0351,0112,0340,0132,0137,0155,
	0171,0201,0202,0203,0204,0205,0206,0207,
	0210,0211,0221,0222,0223,0224,0225,0226,
	0227,0230,0231,0242,0243,0244,0245,0246,
	0247,0250,0251,0300,0152,0320,0241,0007,
	0040,0041,0042,0043,0044,0025,0006,0027,
	0050,0051,0052,0053,0054,0011,0012,0033,
	0060,0061,0032,0063,0064,0065,0066,0010,
	0070,0071,0072,0073,0004,0024,0076,0341,
	0101,0102,0103,0104,0105,0106,0107,0110,
	0111,0121,0122,0123,0124,0125,0126,0127,
	0130,0131,0142,0143,0144,0145,0146,0147,
	0150,0151,0160,0161,0162,0163,0164,0165,
	0166,0167,0170,0200,0212,0213,0214,0215,
	0216,0217,0220,0232,0233,0234,0235,0236,
	0237,0240,0252,0253,0254,0255,0256,0257,
	0260,0261,0262,0263,0264,0265,0266,0267,
	0270,0271,0272,0273,0274,0275,0276,0277,
	0312,0313,0314,0315,0316,0317,0332,0333,
	0334,0335,0336,0337,0352,0353,0354,0355,
	0356,0357,0372,0373,0374,0375,0376,0377,
};
/*eject*/
char	asctoibm[] =
{
	0000,0001,0002,0003,0067,0055,0056,0057,
	0026,0005,0045,0013,0014,0015,0016,0017,
	0020,0021,0022,0023,0074,0075,0062,0046,
	0030,0031,0077,0047,0034,0035,0036,0037,
	0100,0132,0177,0173,0133,0154,0120,0175,
	0115,0135,0134,0116,0153,0140,0113,0141,
	0360,0361,0362,0363,0364,0365,0366,0367,
	0370,0371,0172,0136,0114,0176,0156,0157,
	0174,0301,0302,0303,0304,0305,0306,0307,
	0310,0311,0321,0322,0323,0324,0325,0326,
	0327,0330,0331,0342,0343,0344,0345,0346,
	0347,0350,0351,0255,0340,0275,0137,0155,
	0171,0201,0202,0203,0204,0205,0206,0207,
	0210,0211,0221,0222,0223,0224,0225,0226,
	0227,0230,0231,0242,0243,0244,0245,0246,
	0247,0250,0251,0300,0117,0320,0241,0007,
	0040,0041,0042,0043,0044,0025,0006,0027,
	0050,0051,0052,0053,0054,0011,0012,0033,
	0060,0061,0032,0063,0064,0065,0066,0010,
	0070,0071,0072,0073,0004,0024,0076,0341,
	0101,0102,0103,0104,0105,0106,0107,0110,
	0111,0121,0122,0123,0124,0125,0126,0127,
	0130,0131,0142,0143,0144,0145,0146,0147,
	0150,0151,0160,0161,0162,0163,0164,0165,
	0166,0167,0170,0200,0212,0213,0214,0215,
	0216,0217,0220,0232,0233,0234,0235,0236,
	0237,0240,0252,0253,0254,0255,0256,0257,
	0260,0261,0262,0263,0264,0265,0266,0267,
	0270,0271,0272,0273,0274,0275,0276,0277,
	0312,0313,0314,0315,0316,0317,0332,0333,
	0334,0335,0336,0337,0352,0353,0354,0355,
	0356,0357,0372,0373,0374,0375,0376,0377,
};
/*eject*/
main(argc, argv)
int	argc;
char	**argv;
{
	register int i,c;
	register char *ibp;
	int (*cmode)();
	int craw(), culraw(), cebc(), casc(), cibm(), sinter();

	for(i = 1; i < argc ; i++ ){
		cmd = argv[i];		/* get next command */
		if(eq("if=")){
			infile = cmd;
			continue;
		}
		if(eq("of=")){
			outfile = cmd;
			continue;
		}
		if(eq("ibs=")){
			ibs = getnum();
			continue;
		}
		if(eq("obs=")){
			obs = getnum();
			continue;
		}
		if(eq("bs=")){
			bs = getnum();
			continue;
		}
		if(eq("cbs=")){
			cbs = getnum();
			continue;
		}
		if(eq("skip=")){
			skip = getnum();
			continue;
		}
		if(eq("seek=")){
			seek = getnum();
			continue;
		}
		if(eq("count=")){
			count = getnum();
			continue;
		}
		if(eq("files=")){
			files = getnum();
			continue;
		}
		if(eq("conv=")){
			while(-1){
				/* Set mode flags */
				if(*cmd == '\0'){
					break;
				}
				if(eq(",")){
					continue;
				}
				if(eq("ascii")){
					mode |= ASCII;
					continue;
				}
				if(eq("ebcdic")){
					mode |= EBCDIC;
					continue;
				}
				if(eq("ibm")){
					mode |= IBM;
					continue;
				}
				if(eq("lcase")){
					mode |= LCASE;
					continue;
				}
				if(eq("ucase")){
					mode |= UCASE;
					continue;
				}
				if(eq("swab")){
					mode |= SWAB;
					continue;
				}
				if(eq("noerror")){
					mode |= NOERROR;
					continue;
				}
				if(eq("sync")){
					mode |= SYNC;
					continue;
				}
				fprintf(stderr,"dd: Bad conv. mode: %s\n",cmd);
				exit(1);
			}
			continue;
		}
		fprintf(stderr,"dd: Bad argument: %s\n",cmd);
		exit(1);
	}
/*eject*/
/*
 *	Parameters are ok - set up environment
 */
	cmode = craw;
	if(mode & IBM){
		cmode = cibm;
	}
	if(mode & EBCDIC){
		cmode = cebc;
	}
	if(mode & ASCII){
		cmode = casc;
	}
	if( (cmode == craw) && (mode & (LCASE+UCASE)) ){
		cmode = culraw;
	}
	if(infile){
		infd = open(infile,0);
		if(infd < 0){
			fprintf(stderr,"dd: Cannot open %s\n",infile);
			exit(1);
		}
	} else {
		infd = dup(0);		/* Use standard input */
	}
	if(outfile){
		if( (seek == 0) || ((outfd=open(outfile,0666)) < 0) ){
			outfd = creat(outfile,0666);
		}
		if(outfd < 0){
			fprintf(stderr,"dd: Cannot create/open %s\n",outfile);
			exit(1);
		}
	} else {
		outfd = dup(1);		/* Use standard output */
	}
	if(bs){
		obs = ibs = bs;		/* set input/output blocksize to bs */
	}
	if( (ibs == 0) || (obs == 0) ){
		fprintf(stderr,"dd: Block size must not be zero\n");
		exit(1);
	}
/*
 *	Get memory for buffers 
 */
	inbuff = sbrk(ibs);	/* memory for input buffer */
	/* Check if we need output buffer */
	if( (bs) && (cmode == craw) ){
		outbuff = inbuff;	/* If not - use input buffer */
	} else {
		outbuff = sbrk(obs);
	}
	if( (inbuff == (char *) -1) || (outbuff == (char *) -1) ){
		fprintf(stderr,"dd: Not enough memory\n");
		exit(1);
	}
	if(signal(SIGINT,SIG_IGN) != SIG_IGN){
		signal(SIGINT,sinter);	/* Catch interrupt */
	}
	while(skip--){			/* Skip input records */
		read(infd,inbuff,ibs);
	}
	while(seek--){			/* Position to wanted output record */
		lseek(outfd,(long ) obs,1);
	}
	ibcnt = obcnt = 0;		/* Set input/output buffer cnt */
	ibp = inbuff;			/* Set pointer to input buffer */
	obp = outbuff;			/* and to output buffer */
/*eject*/
	while(-1){			/* Do copy/conversion */
		if(ibcnt-- == 0){
			ibcnt = 0;
			/* End of characters in input buffer - read */
			if( (count == 0) || ((pirec+firec) != count) ){
				/* More to read */
				if(mode & (NOERROR+SYNC)){
					/* If skip err or padding, zero fill */
					for(ibp=inbuff;ibp > inbuff+ibs;ibp++){
						*ibp = 0;
					}
					ibp = inbuff;
				}
				ibcnt = read(infd,inbuff,ibs);
			}
			if(ibcnt == -1){
				/* Error at read */
				if( (mode & NOERROR) == 0 ){
					/* Stop on error */
					fprintf(stderr,
						"dd: Read error on input\n");
					emptybuf();
					recinfo();
					exit(1);
				}
				/* Calculate number of characters read */
				ibcnt = 0;
				for(i = 0; i < ibs; i++){
					if(inbuff[i] !=0){
						ibcnt = i;
					}
				}
				recinfo();	
			}
			if( (ibcnt == 0) && --files <=0){
				/* End of characters/files */
				emptybuf();
				recinfo();
				exit(0);
			}
			if(ibcnt != ibs){
				/* Not a full record */
				/* Increment partial rec count */
				pirec++;
				if(mode & SYNC){
					/* Padding specified */
					ibcnt = ibs;
				}
			} else {
				/* Have a full record */
				firec++;
			}
			ibp = inbuff;	/* set up pointer to buffer */
			i = (ibcnt>>1) & ~1;
			if( (mode & SWAB) && (i) ){
				/* Swap bytes */
				do{
					c = *ibp++;
					ibp[-1] = *ibp;
					*ibp++ = c;
				} while(--i);
			}
			ibp = inbuff;
			if( outbuff == inbuff ){
				/* No conversion - only copy */
				obcnt = ibcnt;
				emptybuf();
				ibcnt = 0;
			}
			continue;
		}
		c = 0;
		c =(*ibp++) & 0377;
		(*cmode) (c);		/* Do conversion */
		continue;
	}
}
/*eject*/
craw(ch)
int ch;
{
	/* Do raw conversion - pure copy */

	*obp++ = ch;
	if(++obcnt >= obs){
		emptybuf();
		obp = outbuff;
	}
}


culraw(ch)
int ch;
{
	/* Do raw upper/lower case conversion */
	register int c;

	c = ch;
	if( (mode & UCASE) && (c >= 'a') && (c <= 'z') ){
		c += 'A' - 'a';
	}
	if( (mode & LCASE) && (c >= 'A') && (c <= 'Z') ){
		c += 'a' - 'A';
	}
	craw(c);
}
/*eject*/
casc(ch)
int ch;
{
	/* Do ASCII conversion */
	register int c;

	c = ebctoasc[ch] & 0377;	/* get ascii char */
	if(cbs == 0){
		/* No conversion buffer */
		culraw(c);		/* just do upper/lower conversion */
		return;
	}
	if(c == ' '){
		/* Have a space */
		spacecnt++;
	} else {
		while(spacecnt > 0){
			/* Put out accumulated spaces */
			craw(' ');
			spacecnt--;
		}
		culraw(c);		/* Convert non space char */
	}
	if(++cbcnt >= cbs){
		/* Conversion buffer full */
		craw('\n');		/* Append LF */
		cbcnt = 0;
		spacecnt = 0;		/* Reset counters */
	}
}
/*eject*/
cebc(ch)
int ch;
{
	/* Convert ASCII to EBCDIC */
	register int c;

	c = ch;
	/* Do upper/lower conversion if specified */
	if( (mode & UCASE) && (c >= 'a') && (c <= 'z') ){
		c += 'A' - 'a';
	}
	if( (mode & LCASE) && (c >= 'A') && (c <= 'Z') ){
		c += 'a' - 'A';
	}
	c = asctoebc[c] & 0377;		/* Get EBCDIC char */
	if(cbs == 0){
		/* No conversion buffer specified */
		craw(c);
		return;
	}
	if(ch == '\n'){
		/* Char was LF - pad with spaces */
		while(cbcnt < cbs){
			craw(asctoebc[' '] & 0377);
			cbcnt++;
		}
		cbcnt = 0;		/* reset counter */
		return;
	}
	if(cbcnt == cbs){
		/* We have exeeded conversion buffer size - truncate */
		trec++;
	}
	cbcnt++;
	if(cbcnt <= cbs){
		/* Only transfer char if within conversion buffer */
		craw(c);
	}
}
/*eject*/
cibm(ch)
int ch;
{
	/* Convert ASCII to IBM-EBCDIC */
	register c;

	c = ch;
	/* Do upper/lower conversion if specified */
	if( (mode & UCASE) && (c >= 'a') && (c <= 'z') ){
		c += 'A' - 'a';
	}
	if( (mode & LCASE) && (c >= 'A') && (c <= 'Z') ){
		c += 'a' - 'A';
	}
	c = asctoibm[c] & 0377;		/* Get EBCDIC char */
	if(cbs == 0){
		/* No conversion buffer specified */
		craw(c);
		return;
	}
	if(ch == '\n'){
		/* Char was LF - pad with spaces */
		while(cbcnt < cbs){
			craw(asctoibm[' '] & 0377);
			cbcnt++;
		}
		cbcnt = 0;		/* reset counter */
		return;
	}
	if(cbcnt == cbs){
		/* We have exeeded conversion buffer size - truncate */
		trec++;
	}
	cbcnt++;
	if(cbcnt <= cbs){
		/* Only transfer char if within conversion buffer */
		craw(c);
	}
}
/*eject*/
emptybuf()
{
	/* Empty output buffer */
	register int i;

	if(obcnt == 0){
		/* No chars in buffer */
		return;
	}
	if(obcnt == obs){
		/* Buffer is full */
		forec++;
	} else {
		/* Output partial record */
		porec++;
	}
	i = write(outfd,outbuff,obcnt);
	if(i != obcnt){
		/* Error at write */
		fprintf(stderr,"dd: Error at write to output\n");
		recinfo();
		exit(1);
	}
	obcnt = 0;
}
/*eject*/
recinfo()
{
	/* Output number of records copied */

	fprintf(stderr,"dd: %u+%u records in\n",firec,pirec);
	fprintf(stderr,"dd: %u+%u records out\n",forec,porec);
	if(trec){
		fprintf(stderr,"dd: %u truncated records\n",trec);
	}
}



sinter()
{
	/* Interrupted by user */
	fprintf(stderr,"dd: *** Interrupted! ***\n");
	recinfo();
	exit(1);
}
/*eject*/
eq(s)
char *s;
{
	/* Check if s is equal to first part of "cmd" */
	register char *cs;

	cs = cmd;
	while(*cs++ == *s){
		if(*s++ == '\0'){
			/* end of string - is equal */
			/* Update global string pointer and return TRUE */
			cmd = --cs;
			return(1);
		}
	}
	if(*s != '\0'){
		/* string was not equal */
		return(0);
	}
	cmd = --cs;
	return(1);
}
/*eject*/
getnum()
{
	/* Convert expression in cmd to a number */
	register char *cs;
	unsigned long n;

	cs = cmd; n = 0;
	while( (*cs >= '0') && (*cs <= '9') ){
		n = n*10 + *cs++ - '0';
	}
	while(-1){
		switch(*cs++){
		case 'w':	
			n *= sizeof(int);
			continue;
		case 'b':
			n *= 512;
			continue;
		case 'k':
			n *= 1024;
			continue;
		case '*':
		case 'x':
			cmd = cs;
			n *= getnum();
		case '\0':
			if( (n > MAXNUMBER) || (n < 0) ){
				fprintf(stderr,
				"dd: Argument %D out of range\n",n);
				exit(1);
			}
			return(n);
		}
	}
}
