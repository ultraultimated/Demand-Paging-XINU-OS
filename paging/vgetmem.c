/* vgetmem.c - vgetmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 * vgetmem  --  allocate virtual heap storage, returning lowest WORD address
 *------------------------------------------------------------------------
 */
WORD	*vgetmem(nbytes)
	unsigned nbytes;
{
	STATWORD ps;    
	struct	mblock	*p, *q, *leftover;

	disable(ps);
	nbytes = (unsigned int) roundmb(nbytes);
	if (nbytes==0 || proctab[currpid].vmemlist->mnext== (struct mblock *) NULL) {
		restore(ps);
		return( (WORD *)SYSERR);
	}
	if( (nbytes> (proctab[currpid].vmemlist)->mlen)) {
                restore(ps);
                return( (WORD *)SYSERR);
        }
	else{
		q = proctab[currpid].vmemlist;
		p = proctab[currpid].vmemlist->mnext;
		while(p != (struct mblock *) NULL){
				leftover = (struct mblock *)( (unsigned)p + nbytes );
				if ( q->mlen > nbytes){
                                        q->mnext = leftover;
                                        q->mlen = q->mlen - nbytes;
                                        restore(ps);
                                        return( (WORD *)p );
				} else if ( q->mlen == nbytes ) {
					q->mnext = leftover;
                                        q->mlen = 0;
                                        restore(ps);
                                        return( (WORD *)p );
				}
			q = p;
			p = p->mnext;
		}
		restore(ps);
		return( (WORD *)SYSERR );
	}
}


