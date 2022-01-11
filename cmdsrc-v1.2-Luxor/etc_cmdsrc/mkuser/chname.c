/* chname.c */

/*
 * This function replaces the file with the name pointed to by the first
 * argument by the file with the name pointed to by the second argument.
 * The name of the resulting file becomes the same as that pointed to
 * by the first argument.
 * The file whose name is pointed to by the second argument will be
 * removed.
 * Zero is returned if everything worked out ok, 1 is returned if the first
 * file can't be removed (unlinked), or 2 is returned if the second file
 * can't be linked to the first name.
 * If 2 is returned, the name of the first file will not exist, but the
 * second file will still remain.
 */

/* 1984-02-29, Peter Andersson, Luxor Datorer AB */

int
chname(name, tmpname)
register	char	*name;		/* file to be replaced            */
register	char	*tmpname;	/* file to replace the above file */
{
	if (unlink(name)) {
		return(1);		/* can't update */
	}
	if (link(tmpname, name)) {
		return(2);		/* can't link */
	}
	unlink(tmpname);
	return(0);			/* OK */
}
