/*
 * set teletype modes
 *
 *	Modification history.
 *
 *	11 Aug 1983	JF00
 *		Added ispeed and ospeed for split speed handling.
 *
 *	28 Nov 1983	LA
 *		Added code to set modem parameters and call login
 *
 *	17 Jan 1984	LA
 *		Code to set modem parameters removed!
 *
 *	
 */

#include <stdio.h>
#include <sgtty.h>
#include <signal.h>
#include <dnix/fcodes.h>

struct	tab {
	char	tname;		/* this table name */
	char	nname;		/* successor table name */
	int	iflags;		/* initial flags */
	int	fflags;		/* final flags */
	int	ispeed;		/* input speed */
	int	ospeed;		/* output speed */
	int	xflags;		/* extra flags for various things */	/*JF00*/
} itab[] = {

#define XF_DTRDROP	1	/* DTR-drop static line protocol */
#define	XF_V21		2	/* Modem conventions (5124 in 300 mode) */
#define	XF_V23		3	/* Modem conventions (5124 in 75/1200 mode) */
#define	XF_MOAUTO	4	/* Modem conventions (5124 automatic) */
#define	XF_MOCTL		0xf	/* Protocol selection */
#define	XF_HUPCL	0x10	/* hangup on close */	/*JF00*/
#define	XF_ALARM	0x20	/* timeout checking */	/*JF00*/
#define	XF_NECHO	0x40	/* no echo */	/*JF00*/

/* 300 baud */

	'0', '0',
	ANYP+RAW+NL1+CR1, ANYP+ECHO+CR1,
	B300, B300,0,	/*JF00*/


/* table '-' -- Console TTY 110 */
	'-', '-',
	ANYP+RAW+NL1+CR1, ANYP+ECHO+CRMOD+XTABS+LCASE+CR1,
	B110, B110,0,	/*JF00*/

/* table '1' -- 150 */
	'1', '1',
	ANYP+RAW+NL1+CR1, EVENP+ECHO+FF1+CR2+TAB1+NL1,
	B150, B150,0,	/*JF00*/

/* table '2' -- 9600 */
	'2', '2',
	ANYP+RAW+NL1+CR1, ANYP+XTABS+ECHO+CRMOD+FF1,
	B9600, B9600,0,	/*JF00*/

/* table -- 1200 */
	'3', '3',
	ANYP+RAW+NL1+CR1, ANYP+XTABS+ECHO+CRMOD+FF1,
	B1200, B1200,0,	/*JF00*/

/* table -- 300 */
	'5', '5',
	ANYP+RAW+NL1+CR1, ANYP+ECHO+CR1,
	B300, B300,0,	/*JF00*/

/* table '4' -- Console Decwriter */
	'4', '4',
	ANYP+RAW, ANYP+ECHO+CRMOD+XTABS,
	B300, B300,0,	/*JF00*/

/* table '6' -- 2400 11/23 line */
	'6', '6',
	ANYP+RAW+NL1+CR1, ANYP+ECHO,
	B2400, B2400,0,	/*JF00*/

/* JF specific line behaviours xyz and abc */
/* table 'a' Automatic modem selects 300 or 75/1200 bauds */
/* assumes that driver sets baudrate at open */
	'a', 'a',
	ANYP+RAW+NL1+CR1, ANYP+XTABS+ECHO+CRMOD+FF1,
	0, 0,XF_HUPCL+XF_ALARM+XF_MOAUTO, /* Let baudrate stand as it is */

/* table 'b' 75/1200 bauds modem */
	'b', 'b',
	ANYP+RAW+NL1+CR1, ANYP+XTABS+ECHO+CRMOD+FF1,
	B75, B1200,XF_HUPCL+XF_ALARM+XF_V23,

/* table 'c' 300 bauds modem */
	'c', 'c',
	ANYP+RAW+NL1+CR1, ANYP+XTABS+ECHO+CRMOD+FF1,
	B300, B300,XF_HUPCL+XF_ALARM+XF_V21,

/* table 'x' -- 4680 terminal exchange (9600 bauds and dtrdrop) */
	'x', 'x',
	ANYP+RAW+NL1+CR1, ANYP+XTABS+ECHO+CRMOD+FF1,
	B9600, B9600,XF_HUPCL+XF_ALARM+XF_DTRDROP,

/* table 'y' 75/1200 bauds and dtrdrop */
	'y', 'y',
	ANYP+RAW+NL1+CR1, ANYP+XTABS+ECHO+CRMOD+FF1,
	B75, B1200,XF_HUPCL+XF_ALARM+XF_DTRDROP,

/* table 'z' 300 bauds and dtrdrop */
	'z', 'z',
	ANYP+RAW+NL1+CR1, ANYP+XTABS+ECHO+CRMOD+FF1,
	B300, B300,XF_HUPCL+XF_ALARM+XF_DTRDROP,

/* table 'u' -- bidirectional UUCP - M002 */
	'u', 'u',
	ANYP+RAW, ANYP+CRMOD+TANDEM+ECHO,
	B1200, B1200,XF_NECHO,	/*JF00*/
};

#define	NITAB	sizeof itab/sizeof itab[0]
 
struct
{
	char	*string;
	int	speed;
} speeds[] = {
	"0",	B0,
	"50",	B50,
	"75",	B75,
	"110",	B110,
	"134",	B134,
	"134.5",B134,
	"150",	B150,
	"200",	B200,
	"300",	B300,
	"600",	B600,
	"1200",	B1200,
	"1800",	B1800,
	"2400",	B2400,
	"4800",	B4800,
	"9600",	B9600,
	"19200",	EXTA,
	"exta",	EXTA,
	"extb",	EXTB,
	0,
};
struct
{
	char	*string;
	int	set;
	int	reset;
} modes[] = {
	"even",
	EVENP, 0,

	"-even",
	0, EVENP,

	"odd",
	ODDP, 0,

	"-odd",
	0, ODDP,

	"raw",
	RAW, 0,

	"-raw",
	0, RAW,

	"cooked",
	0, RAW,

	"-nl",
	CRMOD, 0,

	"nl",
	0, CRMOD,

	"echo",
	ECHO, 0,

	"-echo",
	0, ECHO,

	"LCASE",
	LCASE, 0,

	"lcase",
	LCASE, 0,

	"-LCASE",
	0, LCASE,

	"-lcase",
	0, LCASE,

	"-tabs",
	XTABS, 0,

	"tabs",
	0, XTABS,


	"cbreak",
	CBREAK, 0,

	"-cbreak",
	0, CBREAK,

	"cr0",
	CR0, CR3,

	"cr1",
	CR1, CR3,

	"cr2",
	CR2, CR3,

	"cr3",
	CR3, CR3,

	"tab0",
	TAB0, XTABS,

	"tab1",
	TAB1, XTABS,

	"tab2",
	TAB2, XTABS,

	"nl0",
	NL0, NL3,

	"nl1",
	NL1, NL3,

	"nl2",
	NL2, NL3,

	"nl3",
	NL3, NL3,

	"ff0",
	FF0, FF1,

	"ff1",
	FF1, FF1,

	"bs0",
	BS0, BS1,

	"bs1",
	BS1, BS1,

	"33",
	CR1, ALLDELAY,

	"tty33",
	CR1, ALLDELAY,

	"37",
	FF1+CR2+TAB1+NL1, ALLDELAY,

	"tty37",
	FF1+CR2+TAB1+NL1, ALLDELAY,

	"05",
	NL2, ALLDELAY,

	"vt05",
	NL2, ALLDELAY,

	"tn",
	CR1, ALLDELAY,

	"tn300",
	CR1, ALLDELAY,

	"ti",
	CR2, ALLDELAY,

	"ti700",
	CR2, ALLDELAY,

	"tek",
	FF1, ALLDELAY,

	0,
	};

char	*arg;
struct sgttyb mode;

main(argc, argv)
char	*argv[];
{
	char *mtyp,*tty,*prg,*prgpar;
	register struct tab *tabp;

	if ( (*argv)[0]  != '-' ) 
		setty(argc,argv);
	else
        {
/*
 *   parameters: - ttyxx modemtyp 'param for stty' progtorun parforprog
 */
		if ( argc < 5 )  exit(1);  	/* too few parameters */
		prg = *(argv+argc-2);		/* program to run */
		prgpar = *(argv+argc-1);	/* parameters */
		tty = *++argv;			/* terminal */
		mtyp = *++argv;			/* modem type */
		argc = argc - 4;		/* pars for stty */
		for(tabp = itab; tabp < &itab[NITAB]; tabp++)
			if(tabp->tname == *mtyp) break;
		if(tabp >= &itab[NITAB]) tabp = itab;
/*	
 *		tty=terminal, prg,prgpar=prog to run 
 */
		chown(tty,0,0);
		chmod(tty,0622);
		open(tty,2);
		ioctl(0,SF_CTER,&mode);
		dup(0);
		dup(0);
		if( tabp->xflags & XF_HUPCL) ioctl(0,TIOCHPCL,&mode);
		ioctl(0, TIOCGETP, &mode);
		mode.sg_flags = tabp->fflags;
		mode.sg_ospeed = tabp->ospeed;
		mode.sg_ispeed = tabp->ispeed;
		ioctl(0, TIOCSETP, &mode);
		if( argc > 1) setty(argc,argv);
		if( *prgpar == '-' )
			execl(prg,"login",(char *)0);
		else
			execl(prg,prg,prgpar,(char *)0);
		exit(1);
	};
}


setty(argc,argv)
char *argv[];

{
int thiscmd = 0;
int i;
int prevcmd;
	gtty(1, &mode);
	if(argc == 1) {
		prmodes();
	}
	while(--argc > 0) {

		arg = *++argv;
		prevcmd = thiscmd ; thiscmd = 0 ; /*JF00*/
		if (eq("ek")){
			mode.sg_erase = '#';
			mode.sg_kill = '@';
		}
		if (eq("erase")) {
			if(--argc == 0){
				parerr("erase...");
			}
			if (**++argv == '^')
				mode.sg_erase = (*argv)[1] & 037;
			else
				mode.sg_erase = **argv;
		}
		if (eq("kill")) {
			if(--argc == 0){
				parerr("kill...");
			}
			if (**++argv == '^')
				mode.sg_kill = (*argv)[1] & 037;
			else
				mode.sg_kill = **argv;
		}
		if (eq("gspeed")) {
			mode.sg_ispeed = B300;
			mode.sg_ospeed = B9600;
		}
/* { JF00 ... */
		if (eq("ispeed") || eq("input")) {
			thiscmd = 1 ;
		}
		if (eq("ospeed") || eq("output")) {
			thiscmd = 2 ;
		}
		if(prevcmd && eq("speed")) {
			thiscmd = prevcmd ;
		}
/* ... } JF00*/
		if (eq("hup")) {
			ioctl(1, TIOCHPCL, &mode);
		} else
		for(i=0; speeds[i].string; i++)
/* ... { JF00*/
			if(eq(speeds[i].string)) {
				if(prevcmd!=1)
					mode.sg_ospeed = speeds[i].speed;
				if(prevcmd!=2)
					mode.sg_ispeed = speeds[i].speed;
			}
/* ...} JF00*/
		for(i=0; modes[i].string; i++)
			if(eq(modes[i].string)) {
				mode.sg_flags &= ~modes[i].reset;
				mode.sg_flags |= modes[i].set;
			}
		if(arg)
			fprintf(stderr,"unknown mode: %s\n", arg);
	}
	stty(1,&mode);
}

parerr(s)
register char *s;
{
	fprintf(stderr,"stty: Invalid parameter specification, %s\n",s);
	exit(1);
}

eq(string)
char *string;
{
	int i;

	if(!arg)
		return(0);
	i = 0;
loop:
	if(arg[i] != string[i])
		return(0);
	if(arg[i++] != '\0')
		goto loop;
	arg = 0;
	return(1);
}

prmodes()
{
	register m;

	if(mode.sg_ispeed != mode.sg_ospeed) {
		prspeed("input speed  ", mode.sg_ispeed);
		prspeed("output speed ", mode.sg_ospeed);
	} else
		prspeed("speed ", mode.sg_ispeed);
	if (mode.sg_erase < ' ')
		fprintf(stderr, "erase = '^%c'; ", '@' + mode.sg_erase);
	else
		fprintf(stderr, "erase = '%c'; ", mode.sg_erase);
	if (mode.sg_kill < ' ')
		fprintf(stderr, "kill = '^%c'\n", '@' + mode.sg_kill);
	else
		fprintf(stderr, "kill = '%c'\n", mode.sg_kill);
	m = mode.sg_flags;
	if(m & EVENP)	fprintf(stderr,"even ");
	if(m & ODDP)	fprintf(stderr,"odd ");
	if(m & RAW)	fprintf(stderr,"raw ");
	if(m & CRMOD)	fprintf(stderr,"-nl ");
	if(m & ECHO)	fprintf(stderr,"echo ");
	if(m & LCASE)	fprintf(stderr,"lcase ");
	if((m & XTABS)==XTABS)	fprintf(stderr,"-tabs ");
	if (m & CBREAK)	fprintf(stderr,"cbreak ");
	delay((m&NLDELAY)/NL1,	"nl");
	if ((m&TBDELAY)!=XTABS)
		delay((m&TBDELAY)/TAB1,	"tab");
	delay((m&CRDELAY)/CR1,	"cr");
	delay((m&VTDELAY)/FF1,	"ff");
	delay((m&BSDELAY)/BS1,	"bs");
	fprintf(stderr,"\n");
}

delay(m, s)
char *s;
{

	if(m)
		fprintf(stderr,"%s%d ", s, m);
}

int	speed[] = {
	0,50,75,110,134,150,200,300,600,1200,1800,2400,4800,9600,0,0
};

prspeed(c, s)
char *c;
{

	fprintf(stderr,"%s%d baud\n", c, speed[s]);
}
