/* getpw.c */

/*
 * This routine gets a password, encrypts it and returns a pointer to the
 * encrypted password. NULL is returned if an empty password is entered.
 */

/* 1984-02-17, Peter Andersson, Luxor Datorer AB */

#include	<stdio.h>
#include	"defs.h"
#include	"msg.h"

char *
getpw(cmd)
register	char	*cmd;		/* command name */
{
	char			*getpass();	/* get password function     */
	char			*crypt();	/* encrypt password function */
	char			*strcpy();	/* copy string function      */
	register	char	*passp;		/* pointer to password       */
	char			passav[9];	/* password save             */
	register	int	i, iloop;	/* loop indicies             */
	register	int	flg;		/* flag                      */
	register	int	type;		/* character class           */

	do {				/* until same password */
		for (iloop = 1 ; iloop <= TRIES ; iloop++) {
			passp = getpass(PASSWORD);
			strcpy(passav, passp);
			if (passav[0] == '\0') {
				return(NULL);		/* empty password */
			}
			for (i = 0 ; passav[i] != '\0' ; i++) {
				if (passav[i] < '0' || passav[i] > '9') {
					break;		/* not a digit */
				}
			}
			if (passav[i] == '\0' && iloop < TRIES) {
				cmderr(1, cmd, USENONDIG);
			}
			else {
				type = classify(passav[0]);
				for (i = 1, flg = 0 ; i < MIXEDLEN && passav[i] != '\0' ; i++) {
					flg = (type == classify(passav[i])) ? flg : 1;
				}
				if ((flg || type == NOTALPHA) && i == MIXEDLEN) {
					break;		/* password ok */
				}
				for ( ; passav[i] != '\0' ; i++)
					;
				if (i >= SAMELEN) {
					break;		/* password ok */
				}
				if (iloop < TRIES) {
					cmderr(1, cmd, USELONGER);
				}
			}
		}
		passp = getpass(REPEAT);
		if (flg = strcmp(passp, passav)) {
			cmderr(1, cmd, NOTSAME);
		}
	} while (flg);
	return(crypt(passav, ".."));
}

/*
 * Classify a character.
 */

int
classify(c)
register	c;		/* character to classify */
{
	return((c >= 'A' && c <= 'Z') ? UPPERCASE : ((c >= 'a' && c <= 'z') ? LOWERCASE : NOTALPHA));
}
