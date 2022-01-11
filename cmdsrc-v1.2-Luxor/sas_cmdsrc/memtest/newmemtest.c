#include <stdio.h>
#include <mac.h>
#define K	(1024)
#define M	(K * K)
#define WDOG	{ if(*((char *)0x800007)); }
#define MAP(lpn,fpn)	((*(unsigned short *)(0x800000 +((lpn)<<11))) = ((fpn) | 0x4000))
#define MAXERR	5
#define ABORT	printf("Sub-test aborted!\n"); return;
#define putlong(ad,x)  MAP(512,((ad)>>11)); (*(unsigned long *)(M + ((ad) & 2047))) = x 
#define getlong(ad,x)  MAP(512,((ad)>>11)); x = (*(unsigned long *)(M + ((ad) & 2047))) 
#define putshort(ad,x)  MAP(512,((ad)>>11)); (*(unsigned short *)(M + ((ad) & 2047))) = x 
#define getshort(ad,x)  MAP(512,((ad)>>11)); x = (*(unsigned short *)(M + ((ad) & 2047))) 
#define putbyte(ad,x)  MAP(512,((ad)>>11)); (*(unsigned char *)(M + ((ad) & 2047))) = x 
#define getbyte(ad,x)  MAP(512,((ad)>>11)); x = (*(unsigned char *)(M + ((ad) & 2047))) 

unsigned long sadr,size,value,count;

main(argc,argv)
char **argv;
BEGIN
	IF argc < 4 THEN
		printf("Usage: memtest adr size value <count>\n");
		exit(1);
	FI
	argv++;
	sscanf(*argv,"%X",&sadr);
	argv++;
	sscanf(*argv,"%X",&size);
	argv++;
	sscanf(*argv,"%X",&value);
	IF argc >= 5 THEN
		argv++;
		sscanf(*argv,"%X",&count);
	ELSE
		count = 1;
	FI
	size = size & -4;
	printf("Adr = 0x%X, Size=0x%X, Value=0x%X, Count=0x%X\n",
		sadr,size,value,count);
	fflush(stdout);
	WHILE count-- DO
		printf("Address test...\n");fflush(stdout);
		adrtest();
		printf("Bit test a...\n");fflush(stdout);
		bittest(1,0);
		printf("Bit test b...\n");fflush(stdout);
		bittest(0xfffe,0xffff);
		printf("Byte test...\n");fflush(stdout);
		bytetest();
	OD
END
/*eject*/
adrtest()
BEGIN
	register unsigned long adr,sz,val,vval;
	int errcnt;

	sz = size; adr = sadr; val = value;
	WHILE sz DO
		WDOG;
		putlong(adr,val);
		adr += 4; sz -= 4; val++;
	OD
	sz = 64 * K;
	WHILE sz-- DO WDOG; OD
	errcnt = 0; sz = size; adr = sadr; val = value;
	WHILE sz DO
		WDOG;
		getlong(adr,vval);
		IF val != vval THEN
			err(0,adr,val,vval);
			IF ++errcnt > MAXERR THEN ABORT FI
		FI
		adr += 4; sz -= 4; val++;
	OD
END
/*eject*/
bittest(spat,epat)
unsigned short epat,spat;
BEGIN
	register unsigned long adr,sz;
	register unsigned short pat,ppat;
	int errcnt;

	pat = spat;	errcnt = 0;
	WHILE pat != epat DO
		sz = size; adr = sadr;
		WHILE sz DO
			WDOG;
			putshort(adr,pat);
			adr += 2; sz -= 2;
		OD
		sz = size; adr = sadr;
		WHILE sz DO
			WDOG;
			getshort(adr,ppat);
			IF pat != ppat THEN
				err(1,adr,pat,ppat);
				IF errcnt++ > MAXERR THEN ABORT FI
			FI
			adr += 2; sz -= 2;
		OD
		pat <<= 1;
		IF epat == 0xffff THEN pat |= 1; FI
	OD
END
/*eject*/
bytetest()
BEGIN
	register unsigned long adr,sz;
	register unsigned short val;
	int errcnt;

	errcnt = 0; adr = sadr; sz = size;
	WHILE sz DO
		WDOG;
		putshort(adr,0);
		putbyte(adr,0xff);
		getshort(adr,val);
		IF val != 0xff00 THEN
			err(2,adr,0xff00,val);
			IF errcnt++ > MAXERR THEN ABORT FI
		FI
		putshort(adr,0);
		putbyte(adr+1,0xff);
		getshort(adr,val);
		IF val != 0xff THEN
			err(2,adr,0xff,val);
			IF errcnt++ > MAXERR THEN ABORT FI
		FI
/*eject*/
		putshort(adr,0xffff);
		putbyte(adr,0);
		getshort(adr,val);
		IF val != 0xff THEN
			err(2,adr,0xff,val);
			IF errcnt++ > MAXERR THEN ABORT FI
		FI
		putshort(adr,0xffff);
		putbyte(adr+1,0);
		getshort(adr,val);
		IF val != 0xff00 THEN
			err(2,adr,0xff00,val);
			IF errcnt++ > MAXERR THEN ABORT FI
		FI
		sz -= 2; adr += 2;
	OD
END
/*eject*/
err(type,adr,okval,errval)
register unsigned long okval,errval;
BEGIN
	SWITCH type IN
	CASE 0:
		printf("Address test error, ");
		break;
	CASE 1:
		printf("Bit test error, ");
		break;
	CASE 2:
		printf("Byte test error, ");
		break;
	END
	printf("at phys. adr=%X, read %X, should be %X\n",adr,errval,okval);
	fflush(stdout);
END
