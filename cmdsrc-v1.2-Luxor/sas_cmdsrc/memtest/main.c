/*
 *     S T A N D   A L O N E   M E M O R Y   T E S T E R
 *
 *
 *    Algorithm: The test is divided in two parts, namely:
 *    A bit test, which toggles each bit in an 'environment'
 *    of both ones and zeroes. Ie walking ones ad zeroes.
 *    An addressing ability test, which writes irregular
 *    addressindependant patterns, to test that each byte
 *    can be uniquely addressed. As an option, the operator
 *    may request looping test without any error meesages.
 *    Good for probing around with the scope.
 *
 *    Rev  /  Date   /  who  /  what
 *    1.01 / 840801  /  JL   /  Take off.
 *
 */

#include <stdio.h>

#define TRUE (-1)
#define FALSE 0
#define MAXMEM 2097152
#define ERROR TRUE
#define MAXERR 10
#define WATCHDOG { if(*((char *) 0x800007)); }


typedef unsigned long memoryaddr;

extern end;

int loopflag,errcount;



main(argc,argv)
int argc;
char *argv[];
{
	memoryaddr membot,memtop;
	memoryaddr progstart,progendm;
	memoryaddr stackbot,stacktop;

	char answer;
	char temp[20];

	progstart= (memoryaddr) main;
	progendm = (memoryaddr) &end;
	stackbot = ((memoryaddr) &argc) - 1024;
	stacktop = ((memoryaddr) &argc) + 1024;

	membot = 0;
	memtop = MAXMEM;
	while (TRUE)
	{

		errcount = 0;
		printf("Memory tester R1.01\n");
                printf("Space below 40000H occupied by program and boot.\n");
		do {
			printf("Start address (H)");
			gets(temp);
			sscanf(temp,"%x",&membot);
                        printf("End address   (H)");
			gets(temp);
			sscanf(temp,"%x",&memtop);
		}
		while (membot > memtop);

                printf("Looping test without error log (y/n) ?");
		gets(temp);
		if (temp[0] == 'Y' || temp[0] == 'y' )
			loopflag = TRUE; 						
		else  
			loopflag = FALSE; 						
		do {
			printf("Range %x - %x\n",membot,memtop);
			if (doit(0,progstart,membot,memtop)) {
			} 
			else 
				if (doit(progendm,stackbot,membot,memtop)) {
				} 
				else 
					doit(stacktop,MAXMEM,membot,memtop); 
				
		} 
		while (loopflag);

               if (errcount == 0) printf("-- No errors found --\n");
               printf("Another test (y/n) ?");
               gets(temp);
	       if (temp[0] == 'n' || temp[0] == 'N') exit(1);
	}
}


doit(from,to,membot,memtop)
memoryaddr from,to,membot,memtop;

{
	if ( membot < from )  
		membot = from; 
	if ( memtop > to )  
		memtop = to; 						
	if ( membot > memtop ) 
		return (FALSE);

	if (bittest(membot,memtop))  
		return (ERROR);
	if (addrtest(membot,memtop))  
		return (ERROR);
	return (FALSE);
}


bittest(membot,memtop)

memoryaddr membot,memtop;

{
	unsigned short pattern,phase,lap0,lap1;

	phase = 0;
	for (lap0 = 0; lap0 < 2; lap0++)
	{
		pattern = 1;
		for (lap1 = 1; lap1 < 17; lap1++)
		{
			printf("  Bit test with : ");
			binout(pattern ^ phase);
			printf("\015");
			fill(membot,memtop,pattern ^ phase);
			if (examine(membot,memtop,pattern ^ phase))
				return (ERROR);
			pattern <<= 1;
		}

		phase = ~ phase;
	}



	return (FALSE);
}
/*
 *
 *   Fill memory from start to endm with pattern
 *
 */
fill(start,endm,pattern)

unsigned short *start,*endm,pattern;
{
	unsigned short *address;
	for (address = start; address <= endm; address++)
	{
		*address = pattern;
		WATCHDOG;
	}
}
/*
 *
 *  Examine memory from start to end. Call errorq if <>
 *
 */

examine(start,endm,pattern)
unsigned short *start,*endm,pattern;
{
	unsigned short *address;
	for (address = start; address <= endm; address++)
	{
                if (*address ^ pattern)
			if (errorq(address,pattern,*address))  
				return (ERROR);

		WATCHDOG;
	}
	return (FALSE);
}
/*
 *
 *    Fill and examine with an addressindependant pattern.
 *
 */
  
addrtest(membot,memtop)

unsigned short *membot,*memtop;

{
	unsigned short *addr,xp;
	printf("\n  Address test.\n");
	for (addr = membot; addr <= memtop; addr++)      
	{
		*addr =(unsigned short) addr % 65535;
		WATCHDOG;
	}
	for (addr = membot; addr <= memtop; addr++)
	{
		xp = (unsigned short) addr % 65535;
		if (*addr ^ xp) 
			if (errorq(addr,xp,*addr)) 
				return (ERROR);
		WATCHDOG;
	}
	return (FALSE);
}


errorq(addr,xp,found)

memoryaddr addr;
unsigned short xp,found;

{
	char temp[20];
	if (loopflag == TRUE ) 
		return (FALSE);
        if (errcount == 0) printf("\n");
	printf("Error in %x exp. ",addr);
	binout(xp);
	printf(" found ");
	binout(found);
        icpos(addr,xp,found);
        printf("\n");
	errcount++;
	if (errcount > MAXERR) {
                printf("Shall test continue (y/n/l/q) ?");
		gets(temp);
		if (temp[0] == 'Y' || temp[0] == 'y') { 
			errcount = 0;
			return (FALSE); 
		}
		if (temp[0] == 'L' || temp[0] == 'l') { 
			loopflag = TRUE;
			return (FALSE); 
		}
                if (temp[0] == 'Q' || temp[0] =='q') {
                         exit(1); }
		return (TRUE);
	}
}

/*
 *
 *  Convert to ascii in base 2.
 *
 */
binout(arg)

unsigned short arg;

{
	unsigned short mask;
	for ( mask = 32768; mask != 0; mask >>= 1)
	{
		if ( mask & arg )  
			printf("1");
		else
			printf("0");
	}
}


/*
 *
 * Try to find out which IC that needs replacement.
 * If only one bit differs in xp and found, it may
 * be a good idea, but otherwise it's no use.
 *
 */

icpos(addr,xp,found)
memoryaddr addr;
unsigned short xp,found;

{
        unsigned short pos,row,diff;
        diff = xp ^ found;
        switch (diff) {
        case     1 : pos =  1; break;
        case     2 : pos =  2; break;
        case     4 : pos =  3; break;
        case     8 : pos =  4; break;
        case    16 : pos =  5; break;
        case    32 : pos =  6; break;
        case    64 : pos =  7; break;
        case   128 : pos =  8; break;
        case   256 : pos = 15; break;
        case   512 : pos = 16; break;
        case  1024 : pos = 17; break;
        case  2048 : pos = 18; break;
        case  4096 : pos = 19; break;
        case  8192 : pos = 20; break;
        case 16384 : pos = 21; break;
        case 32768 : pos = 22; break;
        
        default : return (FALSE);
        }
	row = 15 - (( addr & 6 ) / 2 );
       printf(" IC %x %d ?",row,pos);
}

