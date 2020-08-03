/* vfreemem.c - vfreemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 *  vfreemem  --  free a virtual memory block, returning it to vmemlist
 *------------------------------------------------------------------------
 */
SYSCALL	vfreemem(block, size)
	struct	mblock	*block;
	unsigned size;
{
	STATWORD ps;    
	struct	mblock	*p, *q;
	unsigned top;
	struct pentry *pptr;
	
	size = (unsigned)roundmb(size);
	disable(ps);
	if ( size == 0 ){
		restore(ps);
		return(SYSERR);
	}
	else if(((unsigned)block)<((unsigned) &end) ){
		restore(ps);
                return(SYSERR);
	}
	else if(((unsigned)block+size) > (proctab[currpid].vmemlist)->mnext){
		 restore(ps);                                                                                                                  return(SYSERR);
	}
	proctab[currpid].vmemlist->mnext = (((unsigned)block+size) == (proctab[currpid].vmemlist)->mnext)?(unsigned)block : proctab[currpid].vmemlist->mnext - size;
		proctab[currpid].vmemlist->mlen += size; 
		restore(ps);
		return OK;
}
