/* emptychk.c */

/*
 * Check if the string is empty and if so exit the command after
 * displaying a message.
 */

/* 1984-02-20, Peter Andersson, Luxor Datorer AB */

#include	<stdio.h>
#include	"msg.h"

emptychk(buf, cmd)
register	char	*buf;		/* string to check */
register	char	*cmd;		/* command name    */
{
	if (*buf == '\0') {
		cmderr(1, cmd, NONEWUSER);
		exit(0);
	}
	return;
}
