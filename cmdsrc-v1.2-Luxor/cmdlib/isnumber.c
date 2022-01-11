/*	Function for testing if string is a number - isnumber()		*/
/*	=======================================================		*/
/*									*/
/*	Copyright (C) DIAB T{by 1983					*/
/*	Written by: Per-Arne Svensson 					*/

#include <ctype.h>
isnumber(string)
char	*string;
{
register chr;
	while(chr= *string++)
		if (!isdigit(chr))
			return(0);
	return(1);
}
