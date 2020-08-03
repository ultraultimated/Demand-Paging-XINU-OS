/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>
#include <paging.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev;

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	int k = 0, l = 0;
	while(k < proctab[pid].bs_count){
		int s = 0, v = 0;
		int x = bsm_tab[proctab[pid].bs_id[k]].bs_priv;
		if(x != 1){
			while(s < bsm_tab[proctab[pid].bs_id[k]].bs_share_no){
				int m = bsm_tab[proctab[pid].bs_id[k]].bs_share_id[s];
                                if(m != pid){
                                        continue;
					
                                }
				else {
					bsm_unmap(pid, bsm_tab[proctab[pid].bs_id[k]].bs_share_vpno[s], bsm_tab[proctab[pid].bs_id[k]].bs_priv);
					break;
				}
				s++;
                        }
		}
		else{
			bsm_unmap(pid, bsm_tab[proctab[pid].bs_id[k]].bs_vpno, bsm_tab[proctab[pid].bs_id[k]].bs_priv);
		}
		k++;
	}
	int direct = -1;
	do{
		int cas = 0;
		if(frm_tab[l].fr_type == FR_PAGE){
			cas = 1;
		}
		if(frm_tab[l].fr_type == FR_PAGE){
			cas = 2;
		}
		if(cas == 1){
			if(frm_tab[l].fr_pid == pid){
				free_frm(l);
			}
		}
		else if (cas == 2) {
			if(frm_tab[l].fr_pid == pid){
                                direct = l;
			}
		}
		l++;
	} while(l < NFRAMES);
	
	if(direct != -1){
		frm_tab[direct].fr_type = FR_NONE;
		frm_tab[direct].fr_vpno = -1;
		frm_tab[direct].fr_pid = -1;
		frm_tab[direct].fr_refcnt -= 1;
		frm_tab[direct].fr_status = FRM_UNMAPPED;
		delete_frame(direct);
		pd_t *glb_ptt = (pd_t *)((NFRAMES + direct) * NBPG);
		proctab[pid].pdbr = 0;
	}
	
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;

	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
	restore(ps);
	return(OK);
}
