#ifdef	DEBUG
#include <stdio.h>
PRINT(str)
	char	*str;
{
	char	*getvar();
	if( *getvar("DEBUG"))  fprintf(stderr,"%s\n", str);
}
#endif
