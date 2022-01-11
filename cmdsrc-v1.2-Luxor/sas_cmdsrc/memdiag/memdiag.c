/****************************************************************
*		Memory test in single user enviornment		*
*		Checks memory by writting an incremental  	*
*		pattern.				        *
*								*
*****************************************************************/
#include <stdio.h>
#define  k      * 1024
#define  M	* 1024 k 
#define MAXMEM	( 2 M )
#define STACKSIZE ( 2 k )
#define BLOCKSZ ( 2 k )
#define DELAYCNT 512 

typedef unsigned long mem_adr;


/*	Define global variable		*/

char tmpbuff[BLOCKSZ];
extern end;

main(argc,argv)
int argc; 
char *argv[];

{
	mem_adr begmem,endmem;
	mem_adr begprog,endprog;
	mem_adr begstack,endstack;


	begprog = (mem_adr) main;
	endprog = (mem_adr) &end;
	begstack = ( (mem_adr) &argc ) - STACKSIZE/2;
	endstack = ( (mem_adr) &argc ) + STACKSIZE/2;
	begmem = 0;
	endmem = MAXMEM;
	printf("Starting address(in hex):"); 
	scanf("%X",&begmem);
	printf("End address(in hex):     "); 
	scanf("%X",&endmem);
	if( (begmem > endmem) || ((endmem - begmem) > MAXMEM) ){
		printf("Invalid or too big a range to check!\n");
		exit(-1);
	}
	printf("Will not test locations %X-%X (program) and %X-%X (stack)\n",
	begprog,endprog,begstack,endstack);
	dotest(0,begprog,begmem,endmem);
	dotest(endprog,begstack,begmem,endmem);
	dotest(endstack,MAXMEM,begmem,endmem);
}

dotest(first,last,begtest,endtest)
mem_adr first,last,begtest,endtest;
{
	if(begtest < first){
		begtest = first;
	}
	if(begtest > last){
		return;
	}
	if(endtest > last){
		endtest = last;
	}
	if(begtest > endtest){
		return;
	}
	printf("Testing range %X-%X\n",begtest,endtest);
	printf("Bit test...\n");
	data(begtest,endtest);
	printf("\nAddress test...\n");
	address(begtest,endtest);
	printf("\n");
}



data(mstart,mend)
char *mstart,*mend;
{
	unsigned pattern;
	int dot,dotcnt;
	char *mtest;
	char tmp;

	dot = 0;
	dotcnt = 0;
	printf("d");
	for(mtest = mstart; mtest < mend; mtest++){
		if(++dot == DELAYCNT ){
			printf("d");
		 	dot = 0;
			if( ++dotcnt == 64 ){
				printf("\n");
				dotcnt = 0;
			}
		}
		tmp = *mtest;
		for( pattern = 1; pattern <= 128; pattern *=2){
			*mtest=pattern;
			if(test(mtest,pattern)){
				*mtest = tmp;
				exit(1);
			}
		}
		*mtest = tmp;
	}

}



getans()
{
	char s;

	s=getc(stdin);
	if (s == 'e'){
		return(1);
	}
	return(0);
}

address(mstart,mend)
char *mstart,*mend;
{
	char pattern,*tbeg,*adr;
	mem_adr bz,cnt;
	int dot,dotcnt;

	tbeg = mstart;
	bz = 0;
	dotcnt = 0;
	dot = 0;
	for(;;){
		tbeg += bz;
		if( tbeg >= mend){
			return;
		}
		if((tbeg + BLOCKSZ) >= mend){
			bz = mend - tbeg;
		} else {
			bz = BLOCKSZ;
		}
		move(tbeg,tmpbuff,bz);
		printf("a");
		cnt = 0;
		pattern = 0;
		for(adr = tbeg; cnt < bz;){
		if(++dot == DELAYCNT ){
			printf("a");
		 	dot = 0;
			if( ++dotcnt == 64 ){
				printf("\n");
				dotcnt = 0;
			}
		}
			*adr = pattern;
			cnt++;
			adr++;
			if (cnt & 511){
				pattern = (pattern + 1) & 511;
			} else {
				pattern = (pattern + 2) & 511;
			}
		}
		cnt = 0;
		pattern = 0;
		for(adr = tbeg; cnt < bz;){
		if(++dot == DELAYCNT ){
			printf("A");
		 	dot = 0;
			if( ++dotcnt == 64 ){
				printf("\n");
				dotcnt = 0;
			}
		}
			if(test(adr,pattern)){
				move(tmpbuff,tbeg,bz);
				exit(1);
			}
			adr++;
			cnt++;
			if (cnt & 511){
				pattern = (pattern + 1) & 511;
			} else {
				pattern = (pattern + 2) & 511;
			}
		}
		move(tmpbuff,tbeg,bz);
	}
}

move(src,dest,sz)
char *src,*dest;
unsigned long sz;
{
	while(sz--){
		*dest++ = *src++;
	}
}

test(start,pattern)
char *start,pattern;
{
	register rnum;

	rnum = *start;
	if ( rnum != pattern) {
		printf("\n");
		printf(" Failure at location %X \n",start);
		printf(" Wrote %x and Read %x\n",pattern,rnum);
		printf( " Hit key 'e' to exit. 'c' to continue.\n");
		return(getans());
	}
	return(0);
}
