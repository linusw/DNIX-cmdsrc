/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	==========================================================
*
*	== dirlook.c ==		directory lookup
*
*				Look through directory dirname for
*				files matching template filename.
*				Return pointer to WORDlist of pathnames.
*				Let last point to last node in list.
*				IF dironly THEN look for subdirs only.
*
*	==========================================================
*
*	Edit 003	1983-06-29
*/

#include "defs.h"
#include <sys/types.h>		/*  typedefs  */
#include <sys/dir.h>		/*  directory entry structure  */
#include <sys/stat.h>		/*  structure returned by stat  */

extern	char	*getstack();
extern		strcpy();
extern	int	strlen();
extern	int	strequ();
extern	char	*rightstr();
extern	int	wmatch();
extern	char	*unquote();

WORDPTR dirlook(dirname,filename,last,dironly)
char *dirname, *filename;
register WORDPTR *last;
int dironly; 
BEGIN
    
    register FILE *file;
    char pnbuf[256], *pathname = pnbuf;
    register char *chrptr, *rawname = unquote(filename); 
    register struct direct dirbuf;
    struct stat stbuf;
    WORDPTR first = (WORDPTR)NIL;
    
PRINT("DIRLOOK\n");
	IF *dirname == C_NUL THEN dirname = "/" FI
	IF (file = fopen(dirname,"r") ) == NULL THEN			/*  open directory  */
		perror(dirname);
		return( (WORDPTR)NIL );
	FI
    
	strcpy(pathname,dirname);
	chrptr = pathname + strlen(dirname);
	*chrptr++ = '/';
	IF strequ(dirname,"/") THEN ++pathname;
	ELIF strequ(dirname,".") THEN pathname += 2
	FI
    
    first = (WORDPTR)NIL;					/*  initialize list  */
    WHILE (fread((char *)&dirbuf, sizeof(dirbuf), 1, file) ) ==1 DO
        IF !dirbuf.d_ino THEN					/*  slot not in use  */
            continue;
        FI
	BEGIN
		register char *dnp, *pnp; 
		register int i;
		FOR i=0, pnp=chrptr,dnp=dirbuf.d_name; i<DIRSIZ; ++i DO
			*pnp++ = *dnp++;
		OD
		*pnp = C_NUL;
	END        
	IF dironly THEN
		IF stat(pathname, &stbuf) == -1 THEN
			errbrk(pathname); errline(ER_NFND);
			continue;
		FI
		IF (stbuf.st_mode & S_IFMT) != S_IFDIR THEN		/* skip non-directories */
			continue;
		FI
	FI
	IF !strcmp(dirbuf.d_name, rawname) ORF
		wmatch(dirbuf.d_name,filename,TRUE) THEN

            IF !first THEN					/*  put name on list  */
                first = *last = (WORDPTR)getstack(WORDSIZ);
            ELSE
                *last = (*last)->wordnxt = (WORDPTR)getstack(WORDSIZ);
            FI
            (*last)->wordtxt = rightstr(pathname,0);
            (*last)->wordnxt = (WORDPTR)NIL;
        ELSE
        FI
    OD
    
    fclose(file);
PRINT("END DIRLOOK\n");
    return(first);
END
