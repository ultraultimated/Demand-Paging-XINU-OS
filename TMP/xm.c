/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
	STATWORD 	ps;
	disable(ps);
	if(bsm_tab[source].bs_status == BSM_UNMAPPED){
		restore(ps);
		return SYSERR;
	}
	else if(bsm_tab[source].bs_status == BSM_MAPPED){
		if(bsm_tab[source].bs_priv == 1)
		{
			restore(ps);
                	return SYSERR;
		}
		else if (npages > bsm_tab[source].bs_npages){
			restore(ps);
			return SYSERR;
		}
		else{
			int i = 0;
			do{
                        	if(bsm_tab[source].bs_share_id[i] != currpid){
					i++;
					continue;
                        	}
				else{
					if(1){
					proctab[currpid].bs_npages[proctab[currpid].bs_count] = npages;
					proctab[currpid].bs_type[proctab[currpid].bs_count] = 2;
					}
					bsm_tab[source].bs_share_npages[i] = npages;
                                        proctab[currpid].bs_id[proctab[currpid].bs_count] = source;
                                        bsm_tab[source].bs_share_vpno[i] = virtpage;
					proctab[currpid].bs_count++;
					i++;
				}
				
                	}while(i < NPROC);
                restore(ps);
                return OK;
		}
	}
}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{
	STATWORD 	ps;
	disable(ps);
	
	if(bsm_unmap(currpid, virtpage, -1) != OK){
		restore(ps);
		return SYSERR;
	}
  
	restore(ps);
	return OK;
}
