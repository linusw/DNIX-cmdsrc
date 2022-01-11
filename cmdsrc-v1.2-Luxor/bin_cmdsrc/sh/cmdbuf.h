/*
*	ABCenix shell
*
*	Per-Arne Svensson
*	Softec HB
*
*	=========================================================
*
*	== cmdbuf.h ==	include file for command buffer handling
*
*	=========================================================
*
*/


extern	int	infile;
extern	char	*cmdbuf[BUFNUM];	/*  ptrs to command buffers  */
extern	int	readbuf;		/*  index of buffer being read  */
extern	int	getbuf;			/*  index of buffer being filled  */
extern	int	readpos;		/*  where to read in readbuf  */
extern	int	getpos;			/*  where to write in getbuf  */
