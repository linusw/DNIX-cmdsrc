/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	========================================
*
*	== ptrdef.h ==	node pointer definitions
*
*	========================================
*
*	Edit 006	1983-06-26
*/


/*  Pointers to nodes  */

typedef struct node		*NODPTR;
typedef struct fornode		*FORPTR;
typedef struct whilnode		*WHILPTR;
typedef struct untilnode	*UNTILPTR;
typedef struct casenode		*CASEPTR;
typedef struct brchnode		*BRCHPTR;
typedef struct ifnode		*IFPTR;
typedef struct pipenode		*PIPEPTR;
typedef struct andornode	*ANDORPTR;
typedef struct subnode		*SUBPTR;
typedef struct simpnode		*SIMPPTR;
typedef struct asgnnode		*ASGNPTR;
typedef struct wordnode		*WORDPTR;
typedef struct ionode		*IOPTR;
typedef struct varnode		*VARPTR;
typedef	struct intnode		*INTPTR;




/*  Sizes of nodes  */

#define FORSIZ		sizeof(struct fornode)
#define WHILSIZ		sizeof(struct whilnode)
#define UNTILSIZ	sizeof(struct untilnode)
#define CASESIZ		sizeof(struct casenode)
#define BRCHSIZ		sizeof(struct brchnode)
#define IFSIZ		sizeof(struct ifnode)
#define PIPESIZ		sizeof(struct pipenode)
#define ANDORSIZ	sizeof(struct andornode)
#define SUBSIZ		sizeof(struct subnode)
#define SIMPSIZ		sizeof(struct simpnode)
#define ASGNSIZ		sizeof(struct asgnnode)
#define WORDSIZ		sizeof(struct wordnode)
#define IOSIZ		sizeof(struct ionode)
#define VARSIZ		sizeof(struct varnode)
#define	INTSIZ		sizeof(struct intnode)


/*  Type codes for parse-tree nodes  */

#define	NULLTYPE	0
#define FORTYPE		1
#define WHILTYPE	2
#define UNTILTYPE	3
#define CASETYPE	4
#define IFTYPE		5
#define PIPETYPE	6
#define ANDTYPE		7
#define ORTYPE		8
#define SUBTYPE		9
#define SIMPTYPE	10

/*  Type codes for ionodes  */

#define INPTYPE		1
#define OUTTYPE		2
#define INDUPTYPE	3
#define OUTDUPTYPE	4
#define	CLINPTYPE	5
#define	CLOUTTYPE	6
#define APPTYPE		7
#define HDOCTYPE	8
