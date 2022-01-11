/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	================================
*
*	== nodes.h ==	node definitions
*
*	================================
*
*	Edit 009	1983-07-08
*/


/*------------------*/
/*  cmd-tree nodes  */
/*------------------*/

struct node
BEGIN
    int		nodetype;
    NODPTR	nodenxt;
    int		nodeback;
END;

struct fornode
BEGIN
    int		fortype;
    NODPTR	fornxt;
    int		forback;
    char	*forvar;
    WORDPTR	forwlst;
    NODPTR	fordo;
END;
struct whilnode
BEGIN
    int		whiltype;
    NODPTR	whilnxt;
    int		whilback;
    NODPTR	whilcond;
    NODPTR	whildo;
END;

struct untilnode
BEGIN
    int		untiltype;
    NODPTR	untilnxt;
    int		untilback;
    NODPTR	untilcond;
    NODPTR	untildo;
END;

struct casenode
BEGIN
    int		casetype;
    NODPTR	casenxt;
    int		caseback;
    char	*casevar;
    BRCHPTR	caselist;
END;

struct pipenode
BEGIN
    int		pipetype;
    NODPTR	pipenxt;
    int		pipeback;
    NODPTR	pipein;
    NODPTR	pipeout;
END;

struct ifnode
BEGIN
    int		iftype;
    NODPTR	ifnxt;
    int		ifback;
    NODPTR	ifcond;
    NODPTR	ifthen;
    NODPTR	ifelse;
END;

struct andornode
BEGIN
    int		andortype;
    NODPTR	andornxt;
    int		andorback;
    NODPTR	andorleft;
    NODPTR	andorright;
END;

struct subnode
BEGIN
    int		subtype;
    NODPTR	subnxt;
    int		subback;
    int		subsub;
    NODPTR	subdo;
END;

struct simpnode
BEGIN
    int		simptype;
    NODPTR	simpnxt;
    int		simpback;
    ASGNPTR	simpasgn;
    WORDPTR	simpcmd;
    IOPTR	simpio;
END;

struct asgnnode
BEGIN
    ASGNPTR	asgnnxt;
    char	*asgnname;
    char	*asgnval;
END;

/*-------------------*/
/*  Attribute nodes  */
/*-------------------*/

struct brchnode
BEGIN
    BRCHPTR	brchnxt;
    WORDPTR	brchkeys;
    NODPTR	brchdo;
END;

struct wordnode
BEGIN
    WORDPTR	wordnxt;
    char	*wordtxt;
END;

struct ionode
BEGIN
    int		iotype;
    int		iofd1;
    int		iofd2;
    IOPTR	ionxt;
    char	*iotxt;
    char	*iohdoc;
END;

struct varnode
BEGIN
    VARPTR	varnxt;
    char	*varnam;
    char	*varval;
    int		varexp;
    int		varrdonly;
END;


/*  Node for linked list of integers  */

struct intnode
BEGIN
    INTPTR	intnxt;
    int		intval;
END;

/*  Building block for string-to-integer lookup tables  */

struct atoitab
BEGIN
    char	*tabstr;
    int		tabcode;
END;
