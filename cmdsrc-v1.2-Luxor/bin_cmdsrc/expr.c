#include <stdio.h>
#define BYTE_1 unsigned char
#define RS_CY 1
BYTE_1 *evaluate();
/*
*	return structure for subroutines.
*/
struct ret_str {
	int	ret_stat;
	long	(*ret_hnd)();		/* handler address. */
	long	ret_val;		/* returned value. */
};

main(argc,argv)
int argc;
BYTE_1 *argv[];
{
	BYTE_1 buffer[256];
	BYTE_1 *s, *d;
	struct ret_str *rp;
	struct ret_str retblk;
	int i;

	if(argc < 2)
		exit(-1);
	d = buffer;
	for(;argc > 1; --argc) {
		s = argv[1];
		while(*d++ = *s++);
		--d;
		++argv;
	}
	rp = &retblk;
	s = evaluate(buffer, rp);
	if((rp->ret_stat & RS_CY) || (*s != 0)) {
		exit(2);
	}
	printf("%d\n", rp->ret_val);
	if(rp->ret_val)
		exit(0);
	exit(1);
}
/*eject*/
#define PLUS	1
#define MINUS	2
#define MUL	3
#define DIV	4
#define OR	5
#define AND	6
#define XOR	7
#define MOD	8
#define NOT	9
#define NE	10
#define SHL	11
#define SHR	12
#define LT	13
#define LE	14
#define GT	15
#define GE	16
#define EQ	17

BYTE_1 *unary(), *unaryx(), *evalu1();
BYTE_1 *atom(), *expr(), *evalu(), *chkoper();
BYTE_1 *chkoper();
long handlers();
/*
*	operand and handler table.
*/
struct text_str {
	char *text_ops;		/* operands */
	short text_opid;	/* operation id. */
};
struct text_str text0[] = {
	"|",	OR,
	"^",	XOR,
	0,	0
};
struct text_str text1[] = {
	"&",	AND,
	0,	0
};
struct text_str text2[] = {
	"~",	NOT,
	0,	0
};
struct text_str text3[] = {
	"!=",	NE,
	"<",	LT,
	"<=",	LE,
	">",	GT,
	">=",	GE,
	"=",	EQ,
	0,	0
};
struct text_str text4[] = {
	"+",	PLUS,
	"-",	MINUS,
	0,	0
};
struct text_str text5[] = {
	"*",	MUL,
	"/",	DIV,
	"%",	MOD,
	"<<",	SHL,
	">>",	SHR,
	0,	0
};
/*eject*/
/*
*	expression priority table.
*/
struct exptbstr {
	BYTE_1 * (*exp_hnd)();
	struct text_str *exp_ops;	/* pointer to operators and handlers */
} exptab[] = {
	evalu, text0,		/* or and xor */
	evalu, text1,		/* and */
	unary, text2,		/* not */
	evalu, text3,		/* relations. */
	unaryx, text4,		/* unary + and - */
	evalu, text5,		/* *, /, mod, < and > */
	atom, 0			/* atoms */
};
/*eject*/
/*
*	expression evaluator.
*	--------------------
*
*	called with a string pointer and pointer to
*	an area where to leave the result. returns
*	pointer at first untouched character.
*/
BYTE_1 *
evaluate(lptr, retptr)
register BYTE_1 *lptr;
register struct ret_str *retptr;
{

	retptr->ret_stat = 0;		/* clear any status at start. */
	retptr->ret_val = 0;
	retptr->ret_hnd = 0;
	lptr = evalu(lptr,exptab,retptr);
	return(lptr);
}
/*eject*/
/*
*	recursive expression evaluator.
*/
BYTE_1 *
evalu(lptr,exptbptr,retptr)
register BYTE_1 *lptr;
struct exptbstr *exptbptr;
register struct ret_str *retptr;
{
	return(evalu1(lptr,exptbptr,retptr,(short) 0));
}
/*
*	try evaluate unary + and -.
*/
BYTE_1 *
unaryx(lptr,exptbptr,retptr)
BYTE_1 *lptr;
struct exptbstr *exptbptr;
struct ret_str *retptr;
{
	return(evalu1(lptr,exptbptr,retptr,(short) 1));
}
/*eject*/
/*
*
*/
BYTE_1 *
evalu1(lptr,exptbptr,retptr,flag)
register BYTE_1 *lptr;
struct exptbstr *exptbptr;
register struct ret_str *retptr;
short flag;
{
	long lval;		/* left value */
	short opid;		/* operation identifier. */

/*	check for unary + or -. */
	if(flag) {
		lptr = chkoper(lptr,exptbptr->exp_ops,retptr);
		if(retptr->ret_stat & RS_CY) {
			retptr->ret_stat &= ~RS_CY;
		} else {
			lval = 0;
			goto evalu2;		/* unary + or - */
		}
	}
	lptr = expr(lptr,exptbptr,retptr);	/* get left value from next level. */
	if((retptr->ret_stat & RS_CY) == 0) {	/* if no err,lookup operator. */
evalu1:
		lval = retptr->ret_val;		/* left value of expr. */
#ifdef DEBUG
		printf("evaluated value %D\n", lval);
#endif
		lptr = chkoper(lptr,exptbptr->exp_ops,retptr);
		if((retptr->ret_stat & RS_CY) == 0) {
evalu2:
			opid = (short) retptr->ret_val;	/* save opid. */
#ifdef DEBUG
			printf("operation code %d\n", opid);
#endif
			lptr = expr(lptr,exptbptr,retptr);	/* get right value */
			if((retptr->ret_stat & RS_CY) == 0) {
			/* call arithmetical handler. */
				retptr->ret_val = handlers
					(lval,retptr->ret_val,opid,retptr);
				if((retptr->ret_stat & RS_CY) == 0) {
					goto evalu1;
				}
			}
		} else {
/* operator not found, so terminate without error. */
			retptr->ret_stat = 0;
			retptr->ret_val = lval;
		}
	}
	return(lptr);
}
/*eject*/
/*
*	call next levels handlers.
*/
BYTE_1 *
expr(lptr,exptbptr,retptr)
BYTE_1 *lptr;
struct exptbstr *exptbptr;
struct ret_str *retptr;		/* return parameters area. */
{
	struct text_str *pnt;
	exptbptr++;
	lptr = (*exptbptr->exp_hnd)(lptr,exptbptr,retptr);
	return(lptr);
}
/*eject*/
/*
*	subroutine to check operator.
*/
BYTE_1 *
chkoper(lptr,textptr,retptr)
BYTE_1 *lptr;
struct text_str *textptr;		/* pointer to operand table and ops */
struct ret_str *retptr;
{
	register BYTE_1 *s;
	register BYTE_1 *d;

	while(*lptr++ == ' ');
	lptr--;
	retptr->ret_stat = 0;		/* no cy. */
	while(textptr->text_ops) {	/* scan as long we got data */
		d = (BYTE_1 *) textptr->text_ops;	/* point out operands. */
		s = lptr;
		while(*d && (*d == *s)) {
			d++;
			s++;
		}
		if(*d == 0) {		/* operand found. */
			retptr->ret_val = textptr->text_opid;
			return(s);
		}
		textptr++;
	}
	retptr->ret_stat |= RS_CY;	/* mark not found. */
	return(lptr);
}
/*eject*/
/*
*	scan unary operator.
*/
BYTE_1 *
unary(lptr,exptbptr,retptr)
BYTE_1 *lptr;
struct exptbstr *exptbptr;
struct ret_str *retptr;
{
	short opid;
	int stat;

	lptr = chkoper(lptr,exptbptr->exp_ops,retptr);
	opid = (short) retptr->ret_val;
	stat = retptr->ret_stat;
	lptr = evalu(lptr,exptbptr,retptr);	/* get a value */;
	if((retptr->ret_stat & RS_CY) == 0) {	/* error in value? */
		if((stat & RS_CY) == 0) {	/* operator found? */
			retptr->ret_val = handlers
				(retptr->ret_val, (long) 0, opid, retptr);
		}
	}
	return(lptr);
}

/*eject*/
/*
*	scan a constant.
*/
BYTE_1 *
atom(lptr,exptbptr,retptr)
/*
const(lptr,retptr)
*/
BYTE_1 *lptr;
struct exptbstr *exptbptr;
struct ret_str *retptr;
{
	long base = 10;
	register BYTE_1 *s;
	long val, digit;

	retptr->ret_stat = RS_CY;
	s = lptr;
#ifdef DEBUG
	printf("string: %s\n", s);
#endif
	while(*s++ == ' ');		/* skip spaces. */
	--s;
	if(*s == '(') {
		s = evalu(++s,exptab,retptr);
		if((retptr->ret_stat & RS_CY) == 0) {
			while(*s++ == ' ');
			s--;
			if(*s++ != ')') {
				s--;
				retptr->ret_stat |= RS_CY;
			}
		}
	/* first must be digit. */
	} else if ((*s <= '9') && (*s >= '0')) {
		if(*s == '0') {		/* might be hex or octal. */
			s++;
			if(*s == 'x' || *s == 'X') {
				s++;	/* was hex constant. */
				base = 16;
			} else {
				base = 8;
			}
		}
		val = 0;
		while (1) {
			if(*s >= '0' && *s <= '9') {
				digit = (long)(*s -'0');
			} else {
				if(*s >= 'A' && *s <= 'F') {
					digit = (long)(*s - 'A' + 10);
				} else if(*s >= 'a' && *s <= 'f') {
					digit = (long)(*s - 'a' + 10);
				} else {
					digit = 100;	/* will cause error later. */
				}
			}
			if(digit >= base)
				break;
			else {
				val = val*base + digit;	/* collect value. */
				s++;
			}
		}
		retptr->ret_stat &= ~RS_CY;
		retptr->ret_val = val;
	}
	return(s);
}
/*eject*/
/*
*	arithmetical handlers.
*/
long
handlers(lval,rval,operation,retptr)
register long lval;
register long rval;
short operation;
struct ret_str *retptr;
{
	register long result;

	result = (long) 0;
	switch(operation) {
	case PLUS:
		result = lval + rval;
		break;
	case MINUS:
		result = lval - rval;
		break;
	case MUL:
		result = lval * rval;
		break;
	case DIV:
		if(rval) {
			result = lval / rval;
		} else {
			retptr->ret_stat |= RS_CY;
		}
		break;
	case OR:
		result = rval;
		if(lval) result = lval;
		break;
	case AND:
		if(lval && rval) result = lval;
		break;
	case MOD:
		result = lval % rval;
		break;
	case XOR:
		result = lval ^ rval;
		break;
	case NOT:
		result = ~lval;
		break;
	case NE:
		if(lval == rval)
			result++;
		break;
	case SHR:
		result = lval >> rval;
		break;
	case SHL:
		result = lval << rval;
		break;
	case LT:
		if(lval < rval)
			result++;
		break;
	case LE:
		if(lval <= rval)
			result++;
		break;
	case GT:
		if(lval > rval)
			result++;
		break;
	case GE:
		if(lval >= rval)
			result++;
		break;
	case EQ:
		if(lval == rval)
			result++;
		break;
	default:
		printf("expr: unknown operation.\n");
	}
	return(retptr->ret_val = result);
}
