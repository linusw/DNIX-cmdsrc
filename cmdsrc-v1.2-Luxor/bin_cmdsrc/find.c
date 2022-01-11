/*
 *	find - command
 *
 *	Copyright (C) DIAB T{by 1984
 *	Written by: Per-Arne Svensson 
 */

/*
 *	find files
 */

/*
 *	find pathname-list expression
 */

#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <pwd.h>
#include <grp.h>
#include "../cmd.h"
#include "../cmd_err.h"
#define A_DAY	86400L /* a day full of seconds */
#define EQ(x, y)	(strcmp(x, y)==0)
struct anode {
	int (*F)();
	struct anode *L, *R;
};
int	Argc, Ai;
char	**Argv;

/*	External functions						*/
/*	==================						*/
int	setbuf();
char	*strcpy(), *strncpy();	/*	Copy a string			*/
char	*strcat(), *strncat();	/*	Concatenate strings		*/
int	strcmp();		/*	Compare strings			*/
int	strlen();		/*	Length of string		*/
char	*pname();		/*	Get path-name of file		*/
char	*dname();		/*	Get directory-name of file	*/
struct	entry *makentry();	/*	Make one tree entry		*/
instree();			/*	Insert entry in tree structure	*/
fileinfo();			/*	Print info about a file		*/

/*	Stream-I/O buffers						*/
/*	==================						*/
char	findbuf[BUFSIZ];		/*	Output buffer	*/
char	dirbuf[BUFSIZ];			/*	Directory read buffer	*/

/*	Global variables						*/
/*	================						*/
static	struct	stat	statblk;	/*	File status block	*/
time_t	now;				/*	Current time		*/
time_t	new;				/*	Time for newer files	*/
int	Randlast;
char	home[MAXN];			/*	Home path		*/

/*	Local functions							*/
/*	===============							*/

struct	anode	*getanode();	/*	Make a new anode		*/
struct	anode	*exp();		/*	Expression parsing-routines	*/
struct	anode	*e1();
struct	anode	*e2();
struct	anode	*e3();
findtree();			/*	Find descend routine		*/
struct	entry *freetree();	/*	Free memory occupied by tree	*/
struct	stat  *gtstat();	/*	Make one status block entry	*/
char	*nxtarg();		/*	Get next argument string	*/

/*	Main program							*/
/*	============							*/
main(argc, argv)
char *argv[];
int  argc;

{
	struct	anode *extree;		/*	Parsed execution tree	*/
	FILE	*pwd, *popen();
	struct	entry *pathtree = NULL;	/*	Root of path-tree	*/
	struct	entry *entryptr;

	/*	Initiate prompt string					*/
	PRMPT = *argv;
	++argv;
	--argc;

	/*	Assign stream-I/O buffer to output stream		*/
	setbuf(stdout, findbuf);

	/*	Assign stream-I/O buffer to input (directory) stream	*/
	setbuf(stdin, dirbuf);

	/*	Set real UID and effective UID to real UID		*/
	setuid(getuid());

	/*	Fetch current time					*/
	time(&now);

	/*	Fetch home path						*/
	pwd = popen("pwd", "r");
	fgets(home, MAXN, pwd);

	/*	Terminate the home string normally			*/
	home[strlen(home) -1 ] = '\0';


	if (argc < 2) {
		fprintf(stderr,"usage: %s path ... expr ...", PRMPT);
		exit(1);
	}

	/*	Extract the path-list and put it in path tree		*/
	while (*argv && **argv != '-' && **argv != '(' && **argv != '!') {
		if (pathtree)
			instree((entryptr = makentry(pname(*argv), 
						dname(*argv))), pathtree);
		else
			pathtree = entryptr = makentry(pname(*argv), 
						dname(*argv));
		++argv;
		--argc;
	}
	Argv = argv;
	Argc = argc;
	Ai = 0;

	/*	Parse the commands					*/
	if (!(extree = exp())) {
		fprintf(stderr, BADSYN, Argv[Ai]);
		exit(1);
	}

	/*	Check that all arguments are parsed			*/
	if (Ai < argc) {
		fprintf(stderr, MSCONJ);
		exit(1);
	}

	findtree(pathtree, extree);

/*	freetree(pathtree);	*/
	exit(0);
}

/*	compile time functions:  priority is  exp()<e1()<e2()<e3()	*/
/*	==========================================================	*/

struct anode *exp() /* parse ALTERNATION (-o)  */
{
	int or();
	register struct anode * p1;

	p1 = e1() /* get left operand */ ;
	if(EQ(nxtarg(), "-o")) {
		Randlast--;
		return(getanode(or, p1, exp()));
	}
	else if(Ai <= Argc) --Ai;
	return(p1);
}
struct anode *e1() { /* parse CONCATENATION (formerly -a) */
	int and();
	register struct anode * p1;
	register char *a;

	p1 = e2();
	a = nxtarg();
	if(EQ(a, "-a")) {
And:
		Randlast--;
		return(getanode(and, p1, e1()));
	} else if(EQ(a, "(") || EQ(a, "!") || (*a=='-' && !EQ(a, "-o"))) {
		--Ai;
		goto And;
	} else if(Ai <= Argc) --Ai;
	return(p1);
}
struct anode *e2() { /* parse NOT (!) */
	int not();

	if(Randlast) {
		fprintf(stderr, BADSYN, Argv[Ai-1]);
		exit(1);
	}
	Randlast++;
	if(EQ(nxtarg(), "!"))
		return(getanode(not, e3(), (struct anode *)0));
	else if(Ai <= Argc) --Ai;
	return(e3());
}
struct anode *e3() { /* parse parens and predicates */
	int exeq(), ok(), glob(),  mtime(), atime(), xctime(), user(),
		group(), size(), perm(), links(), print(),
		type(), ino(), newer();
	struct anode *p1;
	int i;
	register char *a, *b, s;

	a = nxtarg();
	if(EQ(a, "(")) {
		Randlast--;
		p1 = exp();
		a = nxtarg();
		if(!EQ(a, ")")) goto err;
		return(p1);
	}
	else if(EQ(a, "-print")) {
		return(getanode(print, (struct anode *)0, (struct anode *)0));
	}
	b = nxtarg();
	s = *b;
	if(s=='+') b++;
	if(EQ(a, "-name"))
		return(getanode(glob, (struct anode *)b, (struct anode *)0));
	else if(EQ(a, "-mtime"))
		return(getanode(mtime, (struct anode *)atoi(b), (struct anode *)s));
	else if(EQ(a, "-atime"))
		return(getanode(atime, (struct anode *)atoi(b), (struct anode *)s));
	else if(EQ(a, "-ctime"))
		return(getanode(xctime, (struct anode *)atoi(b), (struct anode *)s));
	else if(EQ(a, "-user")) {
		if((i=IDnum(b, FALSE)) == -1) {
			if(gmatch(b, "[0-9][0-9][0-9]*")
			|| gmatch(b, "[0-9][0-9]")
			|| gmatch(b, "[0-9]"))
				return getanode(user, (struct anode *)atoi(b), (struct anode *)s);
			fprintf(stderr, BADUID, b);
			exit(1);
		}
		return(getanode(user, (struct anode *)i, (struct anode *)s));
	}
	else if(EQ(a, "-inum"))
		return(getanode(ino, (struct anode *)atoi(b), (struct anode *)s));
	else if(EQ(a, "-group")) {
		if((i=IDnum(b, TRUE)) == -1) {
			if(gmatch(b, "[0-9][0-9][0-9]*")
			|| gmatch(b, "[0-9][0-9]")
			|| gmatch(b, "[0-9]"))
				return getanode(group, (struct anode *)atoi(b), (struct anode *)s);
			fprintf(stderr, BADGID, b);
			exit(1);
		}
		return(getanode(group, (struct anode *)i, (struct anode *)s));
	} else if(EQ(a, "-size"))
		return(getanode(size, (struct anode *)atoi(b), (struct anode *)s));
	else if(EQ(a, "-links"))
		return(getanode(links, (struct anode *)atoi(b), (struct anode *)s));
	else if(EQ(a, "-perm")) {
		for(i=0; *b ; ++b) {
			if(*b=='-') continue;
			i <<= 3;
			i = i + (*b - '0');
		}
		return(getanode(perm, (struct anode *)i, (struct anode *)s));
	}
	else if(EQ(a, "-type")) {
		i = s=='d' ? S_IFDIR :
		    s=='b' ? S_IFBLK :
		    s=='c' ? S_IFCHR :
		    s=='f' ? 0100000 :
		    0;
		return(getanode(type, (struct anode *)i, (struct anode *)0));
	}
	else if (EQ(a, "-exec")) {
		i = Ai - 1;
		while(!EQ(nxtarg(), ";"));
		return(getanode(exeq, (struct anode *)i, (struct anode *)0));
	}
	else if (EQ(a, "-ok")) {
		i = Ai - 1;
		while(!EQ(nxtarg(), ";"));
		return(getanode(ok, (struct anode *)i, (struct anode *)0));
	}
	else if(EQ(a, "-newer")) {
		struct stat statblk;
		if(stat(b, &statblk) < 0) {
			fprintf(stderr, NOACC, b);
			exit(1);
		}
		new = statblk.st_mtime;
		return getanode(newer, (struct anode *)0, (struct anode *)0);
	}
err:	fprintf(stderr, UNKNOPR, a);
	exit(1);
}

/*	Function for making a new anode	- getanode()			*/
/*	============================================			*/
/*									*/

struct anode *
getanode(func, left, right)
int (*func)();
struct	anode	*left, *right;
{
	char	*malloc();		/*	Allocate memory		*/
	struct	anode	*thisanode;	/*	Return variable		*/

	thisanode = (struct anode *)malloc(sizeof(struct anode));

	if	(thisanode == NULL) {
	/*	End of memory						*/
		fprintf(stderr, EOFMEM);
		exit(3);
	}

	/*	Initiate pointers					*/
	thisanode->L = left;
	thisanode->R = right;
	thisanode->F = func;
	return(thisanode);
}


char *nxtarg() { /* get next arg from command line */
	static strikes = 0;

	if(strikes==3) {
		fprintf(stderr, BADSYN, "");
		exit(1);
	}
	if(Ai>=Argc) {
		strikes++;
		Ai = Argc + 1;
		return("");
	}
	return(Argv[Ai++]);
}

/* execution time functions */
and(p, name)
register struct anode *p;
register struct entry *name;
{
	return(((*p->L->F)(p->L, name)) && ((*p->R->F)(p->R, name))?1:0);
}
or(p, name)
register struct anode *p;
register struct entry *name;
{
	 return(((*p->L->F)(p->L, name)) || ((*p->R->F)(p->R, name))?1:0);
}
not(p, name)
register struct anode *p;
register struct entry *name;
{
	return( !((*p->L->F)(p->L, name)));
}
glob(p, name)
register struct { int f; char *pat; } *p; 
register struct entry *name;
{
	return(gmatch(dname(name->e_fname), p->pat));
}
print(p, name)
register struct anode *p;
register struct entry *name;
{
	puts(name->e_fname);
	return(1);
}
mtime(p, name)
register struct { int f, t, s; } *p; 
register struct entry *name;
{
	return(scomp((int)((now - statblk.st_mtime) / A_DAY), 
				p->t, p->s));
}
atime(p, name)
register struct { int f, t, s; } *p; 
register struct entry *name;
{
	return(scomp((int)((now - statblk.st_atime) / A_DAY), 
				p->t, p->s));
}
xctime(p, name)
register struct { int f, t, s; } *p; 
register struct entry *name;
{
	return(scomp((int)((now - statblk.st_ctime) / A_DAY), 
				p->t, p->s));
}
user(p, name)
register struct { int f, u, s; } *p; 
register struct entry *name;
{
	return(scomp(statblk.st_uid, p->u, p->s));
}
ino(p, name)
register struct { int f, u, s; } *p;
register struct entry *name;
{
	return(scomp((int)statblk.st_ino, p->u, p->s));
}
group(p, name)
register struct { int f, u; } *p; 
register struct entry *name;
{
	return(p->u == statblk.st_gid);
}
links(p, name)
register struct { int f, link, s; } *p; 
register struct entry *name;
{
	return(scomp(statblk.st_nlink, p->link, p->s));
}
size(p, name)
register struct { int f, sz, s; } *p; 
register struct entry *name;
{
	return(scomp((int)((statblk.st_size+511)>>9), p->sz, p->s));
}
perm(p, name)
register struct { int f, per, s; } *p; 
register struct entry *name;
{
	register i;
	i = (p->s=='-') ? p->per : 07777; /* '-' means only arg bits */
	return((statblk.st_mode & i & 07777) == p->per);
}
type(p, name)
register struct { int f, per, s; } *p;
register struct entry *name;
{
	return((statblk.st_mode&S_IFMT)==p->per);
}
exeq(p, name)
register struct { int f, com; } *p;
register struct entry *name;
{
	fflush(stdout); /* to flush possible `-print' */
	return(doex(p->com, name));
}

ok(p, name)
struct { int f, com; } *p;
register struct entry *name;
{
	int c;  int yes;
	yes = 0;
	fflush(stdout); /* to flush possible `-print' */
	fprintf(stderr, "< %s ... %s >?   ", Argv[p->com], name->e_fname);
	fflush(stderr);
	if((c=getchar())=='y') yes = 1;
	while(c!='\n')
		if(c==EOF)
			exit(2);
		else
			c = getchar();
	if(yes) return(doex(p->com, name));
	return(0);
}

newer(p, name)
register struct { int f, com; } *p;
register struct entry *name;
{
	return(statblk.st_mtime > new);
}


/* support functions */
scomp(a, b, s) /* funny signed compare */
register a, b;
register char s;
{
	if(s == '+')
		return(a > b);
	if(s == '-')
		return(a < (b * -1));
	return(a == b);
}

doex(com, name)
register struct entry *name;
{
	register np;
	register char *na;
	static char *nargv[50];
	static ccode;

	ccode = np = 0;
	while (na=Argv[com++]) {
		if(strcmp(na, ";")==0) break;
		if(strcmp(na, "{}")==0) nargv[np++] = name->e_fname;
		else nargv[np++] = na;
	}
	nargv[np] = 0;
	if (np==0) return(9);
	if(fork()) /*parent*/ wait(&ccode);
	else { /*child*/
		chdir(home);
		execvp(nargv[0], nargv, np);
		exit(1);
	}
	return(ccode ? 0:1);
}

/*	Function for recursive descending in directory tree		*/
/*	===================================================		*/
descend(path, extree)
register struct entry *path;
register struct anode *extree;
{
register struct direct	dir_entry;	/*	Directory entry		*/
register struct entry	*dir_tree = NULL;/*	Root of tree to find	*/
char	*name;				/*	Directory name		*/

	if (path == NULL)
			return(0);

	/*	Test if the entry path-> is a directory			*/
	if (stat((name = dname(path->e_fname)), &statblk) < 0) {
		fprintf(stderr, NOACC, path->e_fname);
		return(1);
	}

	/*	Execute the execution-list				*/
		(*extree->F)(extree, path);

	if	(isdir(statblk)) {

		if (chdir(name) < 0) {
			fprintf(stderr, NOCHDIR, path->e_fname);
			return(1);
		}

		if (freopen(DOT , "r", stdin) == NULL) {
			fprintf(stderr, NOOPEN, path->e_fname);
			return(1);
		}

		/*	Make a new tree and list it			*/
		while (fread((char *)&dir_entry, sizeof(dir_entry), 
			1, stdin) == 1) {

			/*	Test if the file is deleted		*/
			/*	or if it should not be listed		*/
			if (dir_entry.d_ino == 0 ||
				!strcmp(dir_entry.d_name, DOT) ||
				!strcmp(dir_entry.d_name, DOTDOT))
				continue;

			/*	Make a new filename			*/

			if (dir_tree == NULL)
				dir_tree = makentry(path->e_fname, dir_entry.d_name);
			else
				instree(makentry(path->e_fname, dir_entry.d_name), dir_tree);
		}

/*		fclose(stdin); should not be used with freopen()	*/

		/*	Recurse in the tree				*/
		descend(dir_tree, extree);

		/*	Free dynamically allocated space		*/
		freetree(dir_tree);
		dir_tree = NULL;

		/*	Change back directory to parent			*/
		if (chdir(DOTDOT) < 0) {
			fprintf(stderr, NOCHDIR, DOTDOT);
			if (chdir(pname(path->e_fname)) < 0) {
				fprintf(stderr, NOCHDIR, pname(path->e_fname));
				exit(1);
			}
		}
	}

	/*	Recurse in the  tree					*/
	descend(path->left, extree);
	descend(path->right, extree);
	return(0);
}

/*	Function for recursive search in directory trees		*/
/*	================================================		*/
findtree(tree, extree)
register struct entry *tree;
register struct anode *extree;
{
	if (tree == NULL)
			return(0);
	findtree(tree->left, extree);
	findtree(tree->right, extree);

	if (chdir(pname(tree->e_fname)) < 0) {
		fprintf(stderr, NOCHDIR, pname(tree->e_fname));
		return(1);
	}
	/*	Be sure that the left and right pointers are NULL	*/
	/*	so that descend() not will try to recurse on them	*/
	tree->left = tree->right = NULL;

	descend(tree, extree);
	freetree(tree);

	/*	We must now be sure that we are were we started		*/
	if (chdir(home) < 0) {
		fprintf(stderr, NOCHDIR, home);
		exit(1);
	}
	return(0);
}
