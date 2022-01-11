/*
 *	pr [-n] [+n] [-h] [-wn] [-ln] [-t] [-sc] [-m] [-b] file ...
 *
 *	Copyright (C) DIAB T{by 1983
 *	Written by: Per-Arne Svensson 
 */

/*
 *	print one or more files
 *
 *	Updated 840716 /ps	Date also when -h specified,
 *				Title centered (vertically) in page header
 */

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include "../cmd.h"
#include "../cmd_err.h"

/*	Constant definitions						*/
/*	====================						*/

#define	TRAILS		5
#define	TAB_CNT		8

/*	Print buffer							*/
/*	============							*/

char	prbuf[BUFSIZ];		/*	Buffer for stdout		*/

/*	External functions						*/
/*	==================						*/
char	*pname();		/*	Get path-name of file		*/
char	*dname();		/*	Get directory-name of file	*/
time_t	time();			/*	Get current time		*/
char	*ctime();		/*	Convert time to ascii-string	*/
char	*ttyname();		/*	Get tty-name			*/
int	setbuf();		/*	Assign stream I/O buffer	*/
char	*mktemp();		/*	Make temporary file name	*/
off_t	fseek(), ftell();		/*	Position file stream		*/


/*	Global variables						*/
/*	================						*/

FILE	*output = stdout;	/*	Standard output stream		*/
FILE	*filetbl[_NFILE];	/*	Table of open files		*/
FILE	**ftblptr;		/*	Pointer to file table		*/
int	nrofcol	=	1;	/*	Number of columns		*/
int	fstpage	=	1;	/*	First page to print		*/
char	*header;		/*	Header string pointer		*/
char	head_str[MAXWIDTH];	/*	Header string			*/
int	pagwdth	=	80;	/*	Pagewidth			*/
int	pagelen	=	72;	/*	Page length			*/
int	colsep	=	' ';	/*	Column separator		*/
int	mflg	=	FALSE;	/*	Print all files simultaneously	*/
				/*	each in one column		*/
int	bflg	=	FALSE;	/*	Use formfeed instead of spaces	*/
int	hflg	=	FALSE;	/*	Header string is defined	*/
int	trls	=	TRAILS;	/*	Number of heading/trailing lines*/
int	wrpflg	=	TRUE;	/*	Use wrap around by using fseek	*/
				/*	only possible on files		*/
char	tempfile[DIRSIZ];	/*	Temporary file name		*/
unsigned short	mode;		/*	Terminal mode			*/
char	*tty;			/*	Terminal name for output	*/

/*			Functions					*/
/*			=========					*/

int	print();		/*	Print files			*/
char	*getcol();		/*	Get one column string		*/
int	protect();		/*	Protect tty for writing		*/
int	unprot();		/*	Reset tty for shared writing	*/
int	abort();		/*	Call abort and return errorcode	*/


/*	Main program							*/
/*	============							*/
main(argc, argv)
char *argv[];
int  argc;

{
char	*time_str;		/*	Ascii time string pointer	*/
struct	stat	statblk;	/*	File status block		*/
register int result = 0;	/*	Return variable			*/

	/*	Initiate prompt string					*/
	PRMPT = *argv;

	++argv;
	--argc;

	/*	Get terminal name					*/
	tty = ttyname(fileno(stdout));

	/*	Initiate signal routine pointers			*/
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	protect();
	signal(SIGHUP, abort);
	signal(SIGINT, abort);
	signal(SIGQUIT, abort);
	signal(SIGTERM, abort);

	/*	Assign stream-I/O buffer to output stream		*/
	setbuf(stdout, prbuf);

	do {
		while (*argv && (**argv == '-' || **argv == '+')) {
			if (**argv == '-') switch (*++*argv) {
			case 'b':/*	Use formfeed char at end of page*/
				bflg = TRUE;
				break;
			case 'h':/*	Take next argument as header	*/
				header = ++*argv;
				if (!*header) {
					if (++argv)
						header = *argv;
					else
						argc = -1;
					--argc;
				}

				hflg = *header ? TRUE : FALSE;
				break;
			case 'w':	/*	Page width		*/
				pagwdth = atoi(++*argv);
				if (pagwdth <= 0)
					argc = -1;
				break;
			case 'l':	/*	Page length		*/
				pagelen = atoi(++*argv);
				if (pagelen <= 0)
					argc = -1;
				break;
			case 'm':	/*	Print files simultaneously*/
				mflg = TRUE;
				break;
			case 's':	/*	Column separator	*/
				colsep = *++*argv;

			/*	If no character set tab as separator	*/
				if (!colsep)
					colsep = '\t';
				break;
			case 't':	/*	Suppress header/trailer	*/
				trls = atoi(++*argv);
				break;
			default:
				if (isnumber(*argv))
					nrofcol	= atoi(*argv);
				else {
					fprintf(stderr, BADSW, *argv);
					argc = -1;
				}
			}
			else if (**argv == '+') {
				if (isnumber(++*argv))
					fstpage	= atoi(*argv);
				else {
					fprintf(stderr, BADSW, *argv);
					argc = -1;
				}
			} 
			argc--;
			argv++;
		}

		/*	Check pagewidth and pagelength			*/
		if (pagwdth <= 0 || pagelen <= 2*trls+1 || argc < 0) {
			fprintf(stderr, "usage: %s [-n] [+n] [-h] [-wn] [-ln] [-t] [-sc] [-m] [-b] file ...\n", PRMPT);
			exit(1);
		}

		if (!*argv) {
			*filetbl = stdin;
			wrpflg = FALSE;	/*	Do not use fseek on a pipe*/

			strcpy(head_str, "");
			time(&statblk.st_mtime);
			time_str = ctime(&statblk.st_mtime);
			strncat(head_str, time_str+4, 12);
			strcat(head_str, " ");
			strncat(head_str, time_str+20, 4);
			strcat(head_str, " ");
			if(hflg)
				strcat(head_str,header);
			result += print(filetbl, head_str, 
				trls, bflg, colsep, pagelen, 
				pagwdth, fstpage, nrofcol);
		} else {
			ftblptr = filetbl;
			if (mflg) {
				strcpy(head_str, "");
				time(&statblk.st_mtime);
				time_str = ctime(&statblk.st_mtime);
				strncat(head_str, time_str+4, 12);
				strcat(head_str, " ");
				strncat(head_str, time_str+20, 4);
				strcat(head_str, " ");
				if (hflg) {
					strcat(head_str,header);
				}
				while (*argv) {
					if(!hflg){
						strncat(head_str, " ", 
						sizeof(head_str)-strlen(head_str)-1);
						strncat(head_str, *argv, 
						sizeof(head_str)-strlen(head_str)-1);
					}
					if ((*ftblptr = fopen(*argv,"r")) 
					== NULL)
						fprintf(stderr, NOOPEN, *argv);
					else
						ftblptr++;
					argv++;
					argc--;
				}
			} else {
				strcpy(head_str, "");
				stat(*argv, &statblk);
				time_str = ctime(&statblk.st_mtime);
				strncat(head_str, time_str+4, 12);
				strcat(head_str, " ");
				strncat(head_str, time_str+20, 4);
				strcat(head_str, " ");
				if (!hflg) {
					strncat(head_str, *argv, 
					sizeof(head_str)-strlen(head_str)-1);
				} else {
					strcat(head_str,header);
				}

				if ((*ftblptr = fopen(*argv,"r")) 
				== NULL)
					fprintf(stderr, NOOPEN, *argv);
				++argv;
				--argc;
			}
			result += print(filetbl, head_str, 
				trls, bflg, colsep, pagelen, 
				pagwdth, fstpage, nrofcol);

		}
	} while (*argv);
	unprot();
	return(result);
}

/*	Function returning a string representing one column - getcol()	*/
/*	==============================================================	*/
char *
getcol(stream_ptr, ffstr_ptr, size, expand, fcnt_ptr, ffcnt_ptr, do_wrp)
FILE	**stream_ptr;		/*	File input stream pointer	*/
FILE	**ffstr_ptr;		/*	Formfeed table pointer		*/
int	size;			/*	Column size			*/
int	expand;			/*	Expand tabs into spaces		*/
int	*fcnt_ptr;		/*	File count pointer		*/
int	*ffcnt_ptr;		/*	Formfeed semaphore pointer	*/
int	do_wrp;			/*	Use wraparound			*/
{
static	char	col[MAXWIDTH+1];/*	Column and return variable	*/
static	char	col_exp[MAXWIDTH+1];/*	Column expand area		*/
int	limit;			/*	Loop limit			*/
register	int	tab_pos;/*	Character count			*/
register	int	colpos;	/*	Column position			*/
register	int	chr;	/*	Input character			*/
off_t	str_wrp;		/*	Stream wrap position		*/
register int	col_wrp;	/*	Column wrap position		*/
register int	tab_wrp;	/*	Column wrap position		*/

	/*	Test if the stream is closed				*/
	if (*stream_ptr == NULL) {
		colpos = 0;
		while (colpos < size)
			col[colpos++] = ' ';
		col[colpos] = '\0';
		return(col);
	}

	/*	Read one column						*/
	tab_pos = colpos = col_wrp = tab_wrp = 0;
	if (size >= MAXWIDTH)
		size = MAXWIDTH;
	while (tab_pos < size && colpos <= MAXWIDTH) {
			
		if ((chr = getc(*stream_ptr)) == EOF) {
			fclose(*stream_ptr);
			*stream_ptr = NULL;
			*ffstr_ptr = NULL;
			(*fcnt_ptr)--;
			tab_pos = size;
			chr = '\0';
			col_wrp = 0;
		}

		/*	Test if it is a wrap character			*/
		else if (isspace(chr) && do_wrp) {
			col_wrp = colpos;
			tab_wrp = tab_pos;
			str_wrp = ftell(*stream_ptr);
		}

		if (chr == '\f') {	/*	Formfeed		*/
			/*	Simulate end of file for this stream by	*/
			/*	saving the stream pointer in *ffstr_ptr	*/
			/*	and decrease the formfeed semaphore	*/
			/*	When a formfeed has been ejected, it	*/
			/*	will be restored again			*/
			*ffstr_ptr = *stream_ptr;
			*stream_ptr= NULL;
			(*ffcnt_ptr)--;
			chr = '\n';
			col_wrp = 0;
		}

		/*	Put the character in the column buffer		*/
		col[colpos++] = chr;

		/*	If it is a writeable character, increase pos	*/
		if (!iscntrl(chr) || chr == '\t')
			tab_pos++;

		switch (chr) {
		case '\t':

			/*	Calculate tab stop			*/
			tab_pos = ((tab_pos-1) / TAB_CNT + 1) * TAB_CNT;
			break;
		case '\n':

			/*	Do not include line feeds		*/
			/*	but stop reading to this column		*/
			colpos--;;
			tab_pos = size;
			break;
		case 010:		/*	Backspace		*/
			if (col[colpos-1] == '\t') {

				/*	Calculate last tab position	*/
				tab_pos = ((tab_pos-2) / TAB_CNT)*TAB_CNT+1;

				/*	Erase buffer character		*/
				colpos--;
			} else if (!iscntrl(col[colpos-1])) {

				/*	Erase buffer character		*/
				colpos--;
				tab_pos--;
			}
			break;
		}
		}

	/*	Test if any wrap position is found			*/
	if (col_wrp != 0 && do_wrp) {
		colpos = col_wrp;
		tab_pos = tab_wrp;
		fseek(*stream_ptr, str_wrp, 0);
	}

	/*	Terminate the string					*/
	col[colpos] = '\0';

	if (expand) {
		/*	Copy col to col_exp -buffer			*/
		strcpy(col_exp, col);

		tab_pos = colpos = 0;
		while (tab_pos < size && (chr = col_exp[colpos++])) {
			col[tab_pos++] = chr;

			switch (chr) {
			case '\t':
				limit = ((--tab_pos) / TAB_CNT+1)*TAB_CNT;
				while (tab_pos < limit && tab_pos < size)
					col[tab_pos++] = ' ';
				break;
			}
		}
		/*	Fill the rest of the column with spaces		*/
		while (tab_pos < size)
			col[tab_pos++] = ' ';

		/*	Terminate the string				*/
		col[tab_pos] = '\0';
	}
	return(col);
}

/*	Function for printing files - print()				*/
/*	=====================================				*/
print(str_tab, hdr_str, trails, frmfeed, sep_chr,pgln, pgwdth, fstpg, 
	colmns)
FILE	**str_tab;		/*	File stream table		*/
char	*hdr_str;		/*	Header string pointer		*/
int	trails;			/*	No of heading/trailing lines	*/
int	frmfeed;		/*	Use formfeed instead of spaces	*/
int	sep_chr;		/*	Column separator character	*/
int	pgln, pgwdth;		/*	Page length and width		*/
int	fstpg;			/*	First page to be printed	*/
int	colmns;			/*	Number of columns to print	*/
{
FILE	*ffstr_tab[_NFILE]	;/*	Table indicating formfeed	*/
				/*	in one column, concisting of	*/
				/*	the temporary saved file	*/
int	files;			/*	Number of open files		*/
int	ff_cnt;			/*	The number of files that not	*/
				/*	have gotten formfeed character	*/
				/*	since last ejected formfeed,	*/
int	colwdth;		/*	Column width			*/
int	loop_cnt;		/*	Loop variable			*/
int	page_cnt;		/*	Page count			*/
register int	col_cnt;	/*	Loop variable			*/
register int	line_cnt;	/*	Line count			*/
register int	exp_tab;	/*	Expand tabs			*/


	/*	Test how nany files that are open			*/
	for (loop_cnt=0; str_tab[loop_cnt] != NULL; loop_cnt++);
	files = loop_cnt;

	/*	Set 'colmns' to at least the number of open files	*/
	if (files > colmns)
		colmns = files;

	/*	Limit the column count and page width			*/
	if (colmns > _NFILE)
		colmns = _NFILE;

	/*	Be sure that the formfeed table is cleared		*/
	for (loop_cnt=0; loop_cnt<colmns; loop_cnt++)
		ffstr_tab[loop_cnt] = NULL;

	if (colmns > 1)
		exp_tab = TRUE;

	if (pgwdth > MAXWIDTH)
		pgwdth = MAXWIDTH;

	colwdth = (pgwdth-1) / colmns;

	/*	First page to print					*/
	page_cnt = 1;

	/*	If fstpg is greater than first page, assign stdout	*/
	/*	to the nul device					*/
	if (fstpg > page_cnt)
		if ((output = fopen(NULL_DEV, "w")) == NULL) {
			fprintf(stderr, TMNFLS);
			output = stdout;
		}

	/*	Print the files						*/
	/*	===============						*/

	/*	If it is multicolumn printing from one file only, 	*/
	/*	reformat the file					*/
	if (files == 1 && colmns > files)
		files = reformat(str_tab, colmns, colwdth-1, pgln-2*trails-1);

	while (files) {

	/*	Initiate the number of formfeeds required to eject	*/
	/*	the page, depending upon the number of currently	*/
	/*	open files.						*/
	ff_cnt = files;

		/*	If this is the first page to print reassign I/O	*/
		if (page_cnt == fstpg && output != stdout) {
			fclose(output);
			output = stdout;
		}
		/*	Eject a formfeed and print header		*/
		formfeed(frmfeed, 0);

		if (trails){
			for(loop_cnt = trails>>1 ; loop_cnt ;loop_cnt--)
				putc('\n',output);
			fprintf(output, PAGEHDR, hdr_str, page_cnt);
			for(loop_cnt = (trails-1)>>1 ; loop_cnt ;loop_cnt--)
				putc('\n', output);
		}
		/*	Write a page					*/
		line_cnt = 0;
		while (line_cnt < pgln - 2*trails -1) {

			col_cnt = 0;
			while (col_cnt < colmns - 1) {

				/*	Get one column			*/
				fputs(getcol(&str_tab[col_cnt], 
					&ffstr_tab[col_cnt], 
					colwdth -1, exp_tab, 
					&files, &ff_cnt, wrpflg), output);

				/*	Write out column separator	*/
				putc(sep_chr, output);

				/*	Increment column count		*/
				col_cnt++;
			}

			/*	Write out last column			*/
			fputs(getcol(&str_tab[colmns-1], 
				&ffstr_tab[colmns-1], 
				colwdth, exp_tab, 
				&files, &ff_cnt, wrpflg), output);

			/*	Write a 'linefeed' to the output	*/
			putc('\n', output);

			/*	Increment line count			*/
			line_cnt++;
		}

		/*	Write trailer					*/
		for (loop_cnt=0; loop_cnt<trails; loop_cnt++)
		putc('\n', output);

		/*	Increment page count				*/
		page_cnt++;

		/*	Restore the saved stream pointers in ffstr_tab	*/
		/*	that indicates that a formfeed in this column	*/
		/*	has been detected in this page, so that the	*/
		/*	file can be read fraom again			*/
		for (loop_cnt=0; loop_cnt < colmns; loop_cnt++) {
			if (ffstr_tab[loop_cnt] != NULL && 
				str_tab[loop_cnt] == NULL) {
				str_tab[loop_cnt]=ffstr_tab[loop_cnt];
				ffstr_tab[loop_cnt] = NULL;
			}
		}
	}
	return(0);
}

/*	Function for reformatting one file for multi column printing	*/
/*	============================================================	*/

reformat(fil_tab, cols, colw, pgsiz)
FILE	**fil_tab;		/*	File stream table		*/
int	cols;			/*	Number of columns		*/
int	colw;			/*	Column width			*/
int	pgsiz;			/*	Number of lines per page	*/
{
FILE	*file;			/*	Stream pointer, input file	*/
FILE	*ff;			/*	Stream pointer, formfeed	*/
register int	i;		/*	Loop variable			*/
register int	col;		/*	Column number			*/
int	loop;			/*	Semaphore indicating file open	*/
int	ff_c;			/*	Formfeed semaphore		*/

	if (*fil_tab == NULL || pgsiz <= 0)
		return(0);

	/*	Put the stream pointer in the variable 'file' and	*/
	/*	create a temporary file in its place			*/
	file = *fil_tab;

	/*	Reset formfeed file pointer				*/
	ff = NULL;

	/*	Now we have one open stream in **fil_tab, duplicate the	*/
	/*	streams to the number of columns			*/
	while (i < cols) {
		/*	Create a temporary file for the listing		*/
		strcpy(tempfile, TEMP_FILE);
		if (access(mktemp(tempfile), 0) != -1) {
			fprintf(stderr, AGAIN);
			return(0);
		}

		if (close(creat(tempfile, 0600)) == -1 || 
			(fil_tab[i++]=fopen(tempfile, "w+"))==NULL ||
			unlink(tempfile) != 0) {
			fprintf(stderr, NOCREA, tempfile);
			unlink(tempfile);
			return(0);
		}
	}

	/*	Read from the input file and write on the temp files	*/
	loop = 1;	/*	File open semaphore			*/
	while (loop) for (col=0; col<cols; col++)
		for (i=0;i<pgsiz;i++) {
			fputs(getcol(&file, &ff, 
			colw-1, TRUE, &loop, &ff_c, wrpflg), fil_tab[col]);

			/*	Put a new line to the output stream	*/
				putc('\n', fil_tab[col]);

			/*	If a formfeed was found, restore stream	*/
			/*	pointer and write a formfeed to stream	*/
			if (ff != NULL) {
				file = ff;
				ff = NULL;
				putc('\f', fil_tab[col]);
			}
		}

	/*	Reset the streams					*/
	for (i=0; i<cols; i++)
		rewind(fil_tab[i]);

	/*	Return the number of open files				*/
	return(cols);
}




formfeed(use_ff, lines)
int	use_ff;			/*	Use formfeed instead of newlines*/
int	lines;			/*	Number of lines to eject	*/
{
	register int	i;
	if (use_ff)
		fprintf(output, "\f");
	else for (i=0; i<lines; ++i)
		fprintf(output, "\n");
	return;
}

/*	Function setting protection bits for the output device	*/
/*	======================================================	*/
protect()
{
	struct stat out_st;/*	Status buffer for output stream	*/
	if (tty != 0 && stat(tty, &out_st) != NULL) {
		mode = out_st.st_mode & ~S_IFMT;
		chmod(tty, 0600);
	}
}

/*	Function resetting protection bits for the output device*/
/*	========================================================*/
unprot()
{
	if (mode != 0)
		chmod(tty, mode);
}

/*	Function for trapping signals abort				*/
/*	===================================				*/
abort()
{
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	unprot();
	_exit(1);
}

