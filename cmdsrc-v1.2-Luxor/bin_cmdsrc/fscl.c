/*
 *	Check if file system is clean
 *	Syntax:	fscl device
 *	Return: 0 - file system is clean
 *		1 - file system is not clean
 *		2 - open or read error on specified device
 *
 */
/*	84-09-24:  fixed byte swapping............	(pf)
 */
#include <sys/types.h>
#include <sys/sysfile.h>
#include <sys/diskpar.h>
#define BZ	512
#define SYSFOF	(SYSFPTR & (BZ-1))
int dev;
unsigned long *cf;
char buf[BZ];

main(argc,argv)
int argc;
register char *argv[];
{
	if(argc == 1){
		printf("fscl: No device specified\n");
		exit(2);
	}
	if((dev = open(argv[1],0)) < 0){
		printf("fscl: Cannot open device %s\n",argv[1]);
		exit(2);
	}
	if( (lseek(dev,SYSFPTR-SYSFOF,0) < 0) || (read(dev,buf,BZ) !=BZ) ){
		printf("fscl: Error at read\n");
		exit(2);
	}
	cf = (unsigned long int * ) (buf + SYSFOF + sizeof(daddr_t) +
						 sizeof(daddr_t) );
	printf("fscl: File system on device %s is ",argv[1]);
	if(*cf == 0x76543210 || *cf == 0x10325476){
		printf("ok.\n");
		exit(0);
	} else {
		printf("not ok!\n");
		exit(1);
	}
}
