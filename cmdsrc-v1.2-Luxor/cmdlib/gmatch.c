/*
 *
 *	Copyright (C) DIAB T{by 1983
 *	Written by: Per-Arne Svensson 
 */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/dir.h>
#include	"../cmd.h"
#include	"../cmd_err.h"


/* globals (file name generation)
 *
 * "*" in params matches r.e ".*"
 * "?" in params matches r.e. "."
 * "[...]" in params matches character class
 * "[...a-z...]" in params matches a through z.
 *
 */


gmatch(name, mask)
	register char	*name, *mask;
{
	register int	mchr;
	register char	chr;

	if ((mchr = *name++) && (mchr &= STRIP) == 0)
			mchr = 0200;
	switch (chr = *mask++) {

	    case '[':
		{
		register int ok = 0, lrng = 077777;
		while (chr = *mask++) {
			if (chr==']')
				return(ok ? gmatch(name, mask):0);
			else if (chr == '-')
				if (lrng <= mchr && mchr <= (*mask++)) ok++;
			else if (mchr == (lrng = (chr & STRIP))) ok++;

		}
		return(0);
		}

	    default:
		if ((chr & STRIP) != mchr) return(0);

	    case '?':
		return(mchr ? gmatch(name, mask) : 0);

	    case '*':
		if (*mask == 0)  return(1);
		--name;
		while (*name)
		  if (gmatch(name++, mask)) return(1);
		return(0);

	    case 0:
		return(mchr == 0);
	}
}
