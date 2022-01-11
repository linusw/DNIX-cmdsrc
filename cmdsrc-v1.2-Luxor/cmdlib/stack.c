/*
 *	Routines for stack simulation used to push and pop strings
 *	of variable length. To facilitate maintaining strings with 
 *	lengths larger than one, first push the string on the stack
 *	and then the length, then it can be popped with one command.
 *
 *	Copyright (C) DIAB T{by 1984
 *	Written by: Per-Arne Svensson 
 */

#include <stdio.h>
#include <sys/types.h>
#include "../cmd.h"

#define PRINT(x) printf("lstram %u nxtram %u rblkend %u nxtbyt %u rblkstr %u rambuf %u\n",x->lstram,x->nxtram,x->rblkend,x->nxtbyt,x->rblkstr,x->rambuf)

struct	ramblk	{
	struct	ramblk	*lstram;/*	Points to last ramblk in chain	*/
	struct	ramblk	*nxtram;/*	Points to next ramblk in chain	*/
	char	*nxtbyt;	/*	Points to next byte in rambuf	*/
	char	*rblkend;	/*	Points to last byte in rambuf	*/
	char	*rblkstr;	/*	Points to first byte in rambuf	*/
	char	*rambuf;	/*	Ram buffer containing stack data*/
};

struct	ramblk	*ramchain = (struct ramblk*)NULL;/*	List of ramblks	*/

/*	allocram() - Routine for stack simulation
 *	Copyright (C) DIAB T{by 1984
 *	Written by: Per-Arne Svensson 
 */
struct	ramblk	*allocram(ramblk, bufsiz)
register struct	ramblk *ramblk;
int	bufsiz;
{
	register struct	ramblk	*newramblk;
	newramblk = (struct ramblk*)alloc(sizeof(struct ramblk));
	newramblk->rambuf = (char *)alloc(bufsiz);
	/*	If ramblk is not NULL, initialize the link pointer	*/
	if (ramblk != (struct ramblk*) NULL)
		ramblk->nxtram = (struct ramblk*) NULL;
	/*	Initialize the new ramblk				*/
	newramblk->lstram = ramblk;
	newramblk->nxtram = (struct ramblk*) NULL;
	newramblk->rblkend = (newramblk->nxtbyt = newramblk->rblkstr = 
		newramblk->rambuf) + bufsiz;
	return(newramblk);
}


/*	push() - Routine for stack simulation
 *	The routine pushes the data from address onto the stack
 *	and returns the address to the element pushed onto the stack.
 *	Copyright (C) DIAB T{by 1984
 *	Written by: Per-Arne Svensson 
 */

char	*push(address, size)
register char	*address;	/*	Address of element to push	*/
register int	size;		/*	Size of element to push		*/
{
	static	int	ramsize;
	register char	*addr = address;
	register int	siz   = size;
	/*	Test if a new ramblock has to be allocated		*/
	if (ramchain == (struct ramblk*)NULL/*	No ram allocated	*/
	|| (int)(ramchain->rblkend - ramchain->nxtbyt) < size) /* No space*/
	{
		ramsize += STACKINC;
		ramchain = allocram(ramchain, MAX(size, ramsize));
	}
	/*	Put the data on the stack				*/
	while (siz--)
		*ramchain->nxtbyt++ = *addr++;
	return(ramchain->nxtbyt-size);
}



/*	pop() - Routine for stack simulation
 *	The routine fills in the data at address in the same order
 *	as it was pushed and returns the current stack address
 *	after the operation that is at the top of the stack.
 *	Copyright (C) DIAB T{by 1984
 *	Written by: Per-Arne Svensson 
 */

char	*pop(address, size)
register char	*address;	/*	Address of element to pop	*/
register int	size;		/*	Size of element to pop		*/
{
	register char	*addr = address + size;
	register int	siz   = size;
	if (ramchain == (struct ramblk*)NULL)/*	No ram allocated	*/
		return(NULL);
	/*	Test if a ramblock has to be freed			*/
	if ((int)(ramchain->nxtbyt - ramchain->rambuf) < size) /* Free 	*/
	{
		/*	Get last ramblk					*/
		register struct ramblk	*ramblk = ramchain->lstram;
		free((char *)ramchain->rambuf);
 		free((char *)ramchain);
 		if ((ramchain = ramblk) == (struct ramblk*) NULL)
			return(NULL);
	}

	/*	Pop the data from the stack				*/
	while (siz--)
		*--addr = *--ramchain->nxtbyt;
	return(ramchain->nxtbyt);
}