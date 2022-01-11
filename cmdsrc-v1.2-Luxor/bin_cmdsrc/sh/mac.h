/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	======================
*
*	== mac.h ==	macros
*
*	======================
*
*	Edit 006	1983-06-26
*/

#define IF	if(
#define THEN	){
#define ELSE	} else {
#define ELIF	} else if (
#define FI	;}

#define BEGIN	{
#define END	}

#define SWITCH	switch(	
#define IN	){
#define	CASE	case	/* ss */
#define	BREAK	break;	/* ss */
#define DEFAULT	default
#define ENDSW	}

#define FOR	for(
#define WHILE	while(
#define DO	){
#define OD	;}

#define REPEAT	do{
#define UNTIL	}while(!(
#define	DOOMSDAY	(1 == 0) ));
#define DONE	));

#define LOOP	for(;;){
#define POOL	}


#define SKIP	;
#define ANDF	&&
#define ORF	||

#define TRUE	(-1)
#define FALSE	0
#define NIL	((char*)0)


#define MAX(a,b)	((a)>(b)?(a):(b))
#define	LOBYTE(n)	n & 0X00FF
#define	HIBYTE(n)	(n >> 8) & 0X00FF


/*============================ I/O MACROS ===================================*/

		/*	Buffers			*/
#define	BUFLEN	256
#define	BUFNUM	256


                     /*  Return (readbuf:readpos)  */
#define	cmdpos()	(int)(BUFLEN*readbuf+readpos)

		/*  Set readpointer to nth character  */
#define	setcmdpos(pos)	{register int n=pos;readbuf=n/BUFLEN;readpos=n%BUFLEN;}
/*	Debug routine PRINT()						*/
#ifndef	DEBUG
#define	PRINT()
#endif
