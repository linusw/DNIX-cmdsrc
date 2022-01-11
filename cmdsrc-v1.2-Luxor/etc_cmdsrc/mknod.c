/*
 
	NAME
		mknod - Creates special file


	SYNTAX
		mknod name [cb] major minor
	or
		mknod name p


	DESCRIPTION
		Creates a special file named "name". Type is either
			p - pipe (fifo)
			c - character device
			b - block device

		Major and minor are decimal or octal numbers specifying
		the major device type and the minor device

*/
/*eject*/
#include <mac.h>
#include <stdio.h>

main(argc,argv)
register char **argv;
BEGIN
	char *name,*type,*major,*minor;
	int i,mode; FILE *fd;

	IF argc < 3 THEN
		fprintf(stderr,"mknod: Use - mknod name [bcp] major minor\n");
		exit(1);
	FI
	name = *(argv + 1);
	type = *(argv + 2);
	SWITCH *type IN
	CASE 'p': CASE 'P':
	BEGIN
		IF mknod(name,010666,0) NOT_EQ 0 THEN
			fprintf(stderr,"mknod: Unable to mknod %s\n",name);
			exit(1);
		FI
		exit(0);
	END
	CASE 'c': CASE 'C': CASE 'b': CASE 'B':
	BEGIN
		IF (*type EQ 'c') ORF (*type EQ 'C') THEN
			mode = 020666;
		ELSE
			mode = 060666;
		FI
		IF argc < 5 THEN
			fprintf(stderr,"mknod: Invalid major/minor spec.\n");
			exit(1);
		FI
		major = *(argv + 3);
		minor = *(argv + 4);
		i = atoi(major);
		IF (i < 0) ORF (i > 255) THEN
			fprintf(stderr,"mknod: Invalid major number\n");
			exit(1);
		FI
		i = atoi(minor);
		IF (i < 0) ORF (i > 255) THEN
			fprintf(stderr,"mknod: Invalid minor number\n");
			exit(1);
		FI
		IF (fd = fopen(name,"w")) EQ 0 THEN
			fprintf(stderr,"mknod: Unable to create %s\n",name);
			exit(1);
		FI
		fprintf(fd,"%s,%s",major,minor);
		fclose(fd);
		IF chmod(name,mode) NOT_EQ 0 THEN
			fprintf(stderr,"mknod: Cannot chmod %s\n",name);
			unlink(name);
			exit(1);
		FI
		exit(0);
	END
	DEFAULT:
	BEGIN
		fprintf(stderr,"mknod: Invalid type specification\n");
		exit(1);
	END
	ENDSW
END
