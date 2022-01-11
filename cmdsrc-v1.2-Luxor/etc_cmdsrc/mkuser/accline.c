/* accline.c */

/*
 * Get one line from the keyboard (standard input), which isn't too long.
 */

/* 1984-02-20, Peter Andersson, Luxor Datorer AB */

#include	<stdio.h>
#include	"defs.h"
#include	"msg.h"

accline(buf, cmd, prompt)
register	char	*buf;		/* input buffer */
register	char	*cmd;		/* command name */
register	char	*prompt;	/* prompt text  */
{
	register	int	len;	/* length of input line */

	do {				/* until string ok */
		printf(prompt);
		len = getline(buf, BSIZE);
		if (len == EOF) {
			exit(1);	/* exit command */
		}
		else if (len == 0) {
			cmderr(1, cmd, TOOLONGSTRING);
		}
	} while (len <= 0);		/* until string ok */

	return;
}
