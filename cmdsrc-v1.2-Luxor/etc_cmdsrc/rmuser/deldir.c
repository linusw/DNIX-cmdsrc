/* deldir.c */

/*
 * Delete the contents of an entire directory, including the directory
 * itself. Zero is returned if everything worked out ok, else non-zero.
 */

/* 1984-03-12, Peter Andersson, Luxor Datorer AB */

#include	<sys/types.h>
#include	<sys/dir.h>
#include	<sys/stat.h>

int
deldir(name, maxpath)
register	char	*name;		/* pointer to directory name to delete */
register	int	maxpath;	/* max allowed length of pathname      */
{
	struct		direct	dirbuf;	/* directory entry structure   */
	struct		stat	stbuf;	/* file status structure       */
	register	char	*begp;	/* pnter used to make pathname */
	register	char	*endp;	/* pnter used to make pathname */
	register	int	fd;	/* file descriptor             */
	register	int	i;	/* loop index                  */

	begp = name + strlen(name);	/* point to end of pathname */
	if (begp + DIRSIZ + 3 >= name + maxpath) {
		return(1);		/* can't construct pathname */
	}
	if (*(begp - 1) != '/') {	/* if last char not a slash */
		*begp++ = '/';		/* add a slash to dir name */
	}
	*begp = '\0';
	if ((fd = open(name, 0)) == -1) {	/* open directory */
		return(1);			/* couldn't open */
	}

	/*
	 * Delete all the files in the directory.
	 */
	while (read(fd, (char *)&dirbuf, sizeof(dirbuf)) > 0) {
		if (dirbuf.d_ino == 0) {
			continue;	/* slot not in use */
		}
		if (!strcmp(dirbuf.d_name, ".") || !strcmp(dirbuf.d_name, "..")) {
			continue;	/* skip self and parent */
		}
		for (i = 0 , endp = begp ; i < DIRSIZ ; i++) {
			*endp++ = dirbuf.d_name[i];
		}
		*endp = '\0';
		if (stat(name, &stbuf) == -1) {
			return(1);	/* can't get status */
		}
		if ((stbuf.st_mode & S_IFMT) == S_IFDIR) {
			if (deldir(name, maxpath)) {	/* delete dir */
				close(fd);		/* close dir */
				return(1);		/* error */
			}
		}
		else if (unlink(name)) {		/* delete the file */
			close(fd);
			return(1);			/* error */
		}
	}
	close(fd);			/* close the directory */
	*begp++ = '.';
	*begp++ = '.';			/* name of parent link */
	*begp = '\0';
	if (unlink(name)) {		/* remove parent link */
		return(1);		/* error */
	}
	*(--begp) = '\0';		/* name of link to itself */
	if (unlink(name)) {		/* remove link to self */
		return(1);		/* error */
	}
	*(--begp) = '\0';		/* the directory itself */
	if (unlink(name)) {		/* remove directory itself */
		return(1);		/* error */
	}
	if (--begp != name) {		/* if not root directory */
		*begp = '\0';		/* remove last slash */
	}
	return(0);			/* it worked out ok */
}
