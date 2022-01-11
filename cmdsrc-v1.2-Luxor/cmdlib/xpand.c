/*
 *	Copyright (C) DIAB T{by 1984
 *	Written by: Per-Arne Svensson 
 *
 *	xpand.c
 *	=======
 *	Routines for file name expansion:
 *	A file tree is delivered to xpandtree(), or a file entry 
 *	to xpandname().
 *	These routines are mutually recursive and returns either
 *	an expanded tree, or the original one if no matches were
 *	found in the paths containing substitution characters.
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <ctype.h>
#include "../cmd.h"
#include "../cmd_err.h"

/*
 *	Function for expansion of one single entry - xpandname()
 *	========================================================
 */

struct	entry	*
xpandname(entryptr)
struct entry	*entryptr;
{
	static	char	namebuf[MAXN];	/*	Name buffer		*/
	struct	entry	*makentry();	/*	Make a filename entry	*/
	struct	entry	*xpndtree();	/*	Expand an entire tree	*/
	struct	entry	*newtree;	/*	Tree of matched names	*/

	/*	If there are file name generation characters in the	*/
	/*	entry-name, open the directory containing the name	*/
	/*	which has substitution characters and expand the	*/
	/*	path into a new one for every new match that are	*/
	/*	found in that directory.				*/
	/*	Go from left to right when expanding and make new path	*/
	/*	names by adding the matched name to the path and then	*/
	/*	adding the tail, if there is any.			*/
	/*	If there is any tail of the expanded paths, check if	*/
	/*	the tail contains any file name generation characters	*/
	/*	and if so invoke the routine xpndtree() which will	*/
	/*	expand the entire tree of files.			*/
	/*	If the possible tail does not contain any file name	*/
	/*	generation characters there is no need to dive down	*/
	/*	into the tree and then detect that for every single	*/
	/*	entry so return the new tree to the invocing subroutine	*/
	/*	or if the tree is NULL, there was not any matches,	*/
	/*	return the entry which was passed as an argument to	*/
	/*	this routine.						*/

	register char	chr, *chrptr, *chr1ptr;
	register int	match;
	char	*path, *pathend, *name, *namend;

	path = name = chrptr = entryptr->e_fname;
	pathend = namend = NULL;
	match = FALSE;

	/*	While no file name generation characters in the entry	*/
	/*	name,  update the pathend character pointer on every	*/
	/*	occurrance of the directory delimiter DELIM and set	*/
	/*	the name pointer to the next character following it.	*/

	while ((chr = *chrptr) && !(match = is_fng(chr)))
		if (chr == DELIM) pathend = (name = chrptr) -1;
	
	/*	If no file name generation characters were found in	*/
	/*	the entry name, return the entry.			*/

	if (!match)
		return(entryptr);

	/*	Search for end of name to substitute for.		*/

	while((chr = *chrptr++) && chr != DELIM);

	/*	If a DELIM was found, substitute it with '\0'		*/
	/*	to end the name string.					*/

	if (chr) *(namend = --chrptr) = '\0';

	/*	If pathend is not null put a zero there to end the path	*/

	/*	Test if path is a directory. If not restore the entry	*/
	/*	and return.						*/
	/*	If pathend is NULL no path is found and the current	*/
	/*	directory should be used.				*/

	if (pathend) {
		struct stat statblk;
		*pathend = '\0';
		if (stat(path, &statblk) < 0 || !isdir(statblk)) {
			*pathend = DELIM;
			if (namend) *namend = DELIM;
			return(entryptr);
		}
	}

	/*	Try to open thye directory				*/

	{
		FILE	*dirstr;
		register struct	direct dir_entry;

		if ((dirstr = fopen(pathend ? path : DOT)) != NULL) {

			/*	Make a new tree if any matches found	*/

			while (fread((char *)&dir_entry, 
				sizeof(dir_entry), 1, dirstr) == 1) {
				/*	Test if the file is deleted	*/
				/*	or if it should be ignored	*/
				if (dir_entry.d_ino == 0
					|| strcmp(dir_entry.d_name, DOT)
					|| strcmp(dir_entry.d_name, DOTDOT))
					continue;
 
				/*	Try to match the names		*/

				if (gmatch(dir_entry.d_name, name)) {

					/*	Copy the path to namebuf*/

					register char *chrptr1 = namebuf;
					register char *endbuf = namebuf + sizeof(namebuf);
					chrptr = path;

					while ((*chrptr1++ = *chrptr++)
						&& chrptr1 != endbuf);

					chrptr = dir_entry.d_name;
					chrptr1--;

					while ((*chrptr1++ = *chrptr++)
						&& chrptr1 != endbuf);

					/*	If name is not the last	*/
					/*	name in the path	*/

					if (chr) {
						chrptr = namend + 1;
						chrptr1--;

						while ((*chrptr1++ = *chrptr++)
							&& chrptr1 != endbuf);
					}
					*chrptr1 = '\0';

					/*	Test if end of buffer	*/

					if (chrptr1 == endbuf) {
						fprintf(stderr, LNGNAM, 
							namebuf);
						freetree(newtree);
						if (pathend)
							*pathend = DELIM;
						if (namend)
							*namend = DELIM;
						return(entryptr);
					} else if (newtree)
						instree(makentry(namebuf, ""), 
							newtree);
					else newtree = makentry(namebuf, "");
				}
			}

			/*	Close the directory			*/

			fclose(dirstr);

		} else {
			fprintf(stderr, NOOPEN, pathend ? path : DOT);
			return(entryptr);
		}
	}

	/*	Test if recursion should be made			*/

	if (chr) {
		chrptr = namend +1;
		match = FALSE;

		while ((chr = *chrptr++) && !(match = is_fng(chr)));
	}

	if (match)
		newtree = xpndtree(newtree);

	return(newtree);
}
