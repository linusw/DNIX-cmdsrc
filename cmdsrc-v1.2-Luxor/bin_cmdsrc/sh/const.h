/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	=======================================
*
*	== const.h ==	miscellaneous constants
*
*	=======================================
*
*	Edit 004	1983-07-19
*/

/*  System parameters  */

#define	FNAMSIZ	14	/* max no. of chars in filename */
#define	FDMIN	0	/* lowest  legal file descriptor */
#define	FDMAX	15	/* highest legal file descriptor */
#define	PMODE	0644	/* create mode :  rw-r--r--  */

/*  System error codes  (errno)  */
#include <errno.h>


/*  Shell flags  */

#define	E_FLAG	1
#define	I_FLAG	2
#define	K_FLAG	4
#define	N_FLAG	8
#define	R_FLAG	16
#define	S_FLAG	32
#define	T_FLAG	64
#define	U_FLAG	128
#define	V_FLAG	256
#define	X_FLAG	512
#define	C_FLAG	1024

/*  Miscellaneous constants  */

#define	TOP	0
#define SUBCMD	1
#define SUBCAS	2
#define	EXPORT  1
#define	RDONLY	2
