/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
static unsigned long esp;
*/

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{
	STATWORD 	ps;
	disable(ps);
	struct	pentry	*pptr1;
	int avil;
	if(get_bsm(&avil) != SYSERR){
		int k = 0;
		int procid = create(procaddr,ssize,priority,name,nargs,args);
		while(!k){
		pptr1 = &proctab[procid];
		pptr1->store = avil;
		pptr1->vhpno = 4096;
		pptr1->vhpnpages = hsize;
		k = 1;
		}
		k = 0;
		while(!k){
		pptr1->bs_npages[pptr1->bs_count] = hsize;
		pptr1->vmemlist->mlen = hsize * 4096;
                pptr1->bs_id[pptr1->bs_count] = avil;
		pptr1->vmemlist->mnext = 4096*4096;
		pptr1->bs_type[pptr1->bs_count] = 1;
		pptr1->bs_count++;
		k = 1;
		}
                if(bsm_map(procid, pptr1->vhpno, pptr1->store, pptr1->vhpnpages) != SYSERR){
                	bsm_tab[avil].bs_priv = 1;
                	restore(ps);
                	return procid;
                }
        	else{
		        restore(ps);
                        return SYSERR;
		}
	}
	else {
		restore(ps);
                return SYSERR;
	}
}
