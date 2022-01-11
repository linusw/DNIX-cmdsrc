/*	Function for making a new password - mk_pwd()			*/
/*	=============================================			*/
/*									*/
/*	Copyright (C) DIAB T{by 1984					*/
/*									*/
#include <sys/types.h>

char	*
mk_pwd(name)
char	*name;
{
time_t	time();			/*	Get current time		*/
time_t	salt;			/*	Salt for encryption of password	*/
char	*crypt();		/*	Encryption routine		*/
char	saltc[2];		/*	Salt array for encryption	*/
register char chr;
register int i;
	/*	Create a salt for the encryption of the 		*/
	/*	password, that lies in [0-9], [a-z] and [A-Z]		*/

	time(&salt);
	salt += getpid();
	saltc[0] = salt & 077;
	saltc[1] = (salt >> 6) & 077;
	for (i=0; i<sizeof(saltc); i++) {
		chr = saltc[i] + '.';
		if (chr>'9') chr += 7;
		if (chr>'Z') chr += 6;
		saltc[i] = chr;
	}

	/*	Encrypt password name					*/
	return(crypt(name, saltc));
}
