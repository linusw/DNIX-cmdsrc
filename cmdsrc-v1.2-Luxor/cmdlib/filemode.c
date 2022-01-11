/*	filemode - get file-type modes					*/
/*	==============================					*/
/*									*/
/*	Copyright (C) DIAB T{by 1983					*/
/*	Written by: Per-Arne Svensson 					*/

#include <sys/types.h>
#include <sys/stat.h>

struct	mode {			/*	File modes			*/
	char	m_ftype;	/*	File-type indication		*/
	char	m_usrrd;	/*	User read permission		*/
	char	m_usrwr;	/*	User write permission		*/
	char	m_usrex;	/*	User execute permission		*/
	char	m_grprd;	/*	Group read permission		*/
	char	m_grpwr;	/*	Group write permission		*/
	char	m_grpex;	/*	Group execute permission	*/
	char	m_othrd;	/*	Others read permission		*/
	char	m_othwr;	/*	Others write permission		*/
	char	m_othex;	/*	Others execute permission	*/
	char	m_end;		/*	Termination of string		*/
};

char *
filemode(statblk)
struct	stat	*statblk;
{
	static	struct	mode f_mode;
	register unsigned short	s_mode;

		f_mode.m_ftype = '-';
		f_mode.m_usrrd = '-';
		f_mode.m_usrwr = '-';
		f_mode.m_usrex = '-';
		f_mode.m_grprd = '-';
		f_mode.m_grpwr = '-';
		f_mode.m_grpex = '-';
		f_mode.m_othrd = '-';
		f_mode.m_othwr = '-';
		f_mode.m_othex = '-';
		f_mode.m_end   = 0;

	s_mode = statblk->st_mode;

	switch(s_mode & S_IFMT) {
	case S_IFDIR:
		f_mode.m_ftype = 'd';
		break;
	case S_IFBLK:
		f_mode.m_ftype = 'b';
		break;
	case S_IFCHR:
		f_mode.m_ftype = 'c';
		break;
	case S_IFIFO:
		f_mode.m_ftype = 'p';
	}

	if (s_mode & S_IREAD)
		f_mode.m_usrrd = 'r';
	if (s_mode & S_IWRITE)
		f_mode.m_usrwr = 'w';
	if (s_mode & S_IEXEC)
		f_mode.m_usrex = 'x';
	if (s_mode & S_ISUID)
		f_mode.m_usrex = 's';
	if (s_mode & S_IREAD>>3)
		f_mode.m_grprd = 'r';
	if (s_mode & S_IWRITE>>3)
		f_mode.m_grpwr = 'w';
	if (s_mode & S_IEXEC>>3)
		f_mode.m_grpex = 'x';
	if (s_mode & S_ISGID)
		f_mode.m_grpex = 's';
	if (s_mode & S_IREAD>>6)
		f_mode.m_othrd = 'r';
	if (s_mode & S_IWRITE>>6)
		f_mode.m_othwr = 'w';
	if (s_mode & S_IEXEC>>6)
		f_mode.m_othex = 'x';
	if (s_mode & S_ISVTX)
		f_mode.m_othex = 't';

	return((char *)&f_mode);
}
