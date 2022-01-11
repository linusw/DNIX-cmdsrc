/* chkpath.c */

/*
 * Check that a pathname is syntactically legal. If so return zero, else
 * non-zero.
 */

/* 1984-02-21, Peter Andersson, Luxor Datorer AB */

int
chkpath(path)
register	char	*path;		/* pathname to check */
{
	while (*path != '\0') {
		if (*path++ != '/' || *path < 'a' || *path > 'z') {
			return(1);	/*  illegal  */
		}
		while ((*path >= '0' && *path <= '9') || *path == '_' ||
			*path == '.' || (*path >= 'a' && *path <= 'z')) {
			path++;
		}
	}
	return(0);
}
