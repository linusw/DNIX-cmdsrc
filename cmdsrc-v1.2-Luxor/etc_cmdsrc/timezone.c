#include <mac.h>
#include <stdio.h>
#include <dnix/types.h>
#include <dnix/bootrec.h>
main(argc,argv)
int	argc;
char	*argv[];
{
	int	timezone;
	int	lu;
	struct bootrec br;

	if(argc > 2){
		fprintf(stderr,"Usage : %s timedifference (in minutes)\n",argv[0]);
		exit(1);
	}
	if(argc > 1 && getuid() != 0){
		fprintf(stderr,"Must be run by superuser\n");
		exit(1);
	}
	if((lu = open("/dev/nvram",argc == 1 ? 0 : 2)) < 0){
		fprintf(stderr,"Can't open /dev/nvram \n");
		exit(1);
	}
	if(read(lu,(char *)&br,sizeof br) != sizeof br){
		fprintf(stderr,"Can't read boot record\n");
		exit(1);
	}
	if(argc == 1){
		/* Print timezone */
		printf("Timezone = %d\n",br.bo_tzone);
		exit(0);
	}
	sscanf(argv[1],"%d",&timezone);
	if(timezone > 12*60 || timezone < -12*60){
		fprintf(stderr,"Illegal timezone \n");
		exit(1);
	}
	printf("Old timezone = %d New timezone = %d\n",br.bo_tzone,timezone);
	br.bo_tzone = timezone;
	br.bo_chsum = -chsum(&br,sizeof(br)-1);
	lseek(lu,0,0);
	if(write(lu,(char *)&br,sizeof br) != sizeof br){
		fprintf(stderr,"Can't write boot record\n");
		exit(1);
	}
	close(lu);
}
chsum(cp,cnt)
BEGIN
	return(chsum1(cp,cnt,0)) ;
END

chsum1(cp,cnt,c)
register char *cp ;
register int cnt ;
register unsigned c ;
BEGIN
	WHILE cnt DO
		cnt-- ;
		c+=(c>255)?((*cp++ & 0377)-255):*cp++ & 0377 ;
	OD
	IF c>255 THEN c-=255 FI
	return(c) ;
END
