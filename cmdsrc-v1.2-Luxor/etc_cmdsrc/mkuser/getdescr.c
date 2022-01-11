/* getdescr.c */

/*
 * Get an optional user description from the keyboard (standard input).
 * Check that no colons are present in the string.
 * Return a pointer to the description.
 */

/* 1984-02-20, Peter Andersson, Luxor Datorer AB */

#include	"msg.h"

char *
getdescr(buf, cmd)
char			*buf;	/* optional user description */
register	char	*cmd;	/* command name              */
{
	register	int	i;	/* loop index */

	do {				/* until ok */
		accline(buf, cmd, OPTDESCR);
		for (i = 0 ; *(buf + i) != '\0' ; i++) {
			if (*(buf + i) == ':') {
				cmderr(1, cmd, NOCOLONS);
				break;
			}
		}
	} while (*(buf + i) == ':');
	return(buf);
}
