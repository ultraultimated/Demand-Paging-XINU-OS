/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
 
bs_map_t bsm_tab[NO_BSM];
SYSCALL init_bsm()
{
	STATWORD 	ps;
	disable(ps);
	 int i = 0,j;
	while(i < NO_BSM){
		bsm_tab[i].bs_share_no = 0;
		bsm_tab[i].bs_vpno = 4096;
		
		bsm_tab[i].bs_status = BSM_UNMAPPED; 
		bsm_tab[i].bs_id = i;
		bsm_tab[i].bs_priv = 0;
                if(1){
		bsm_tab[i].bs_sem = 1; 
		bsm_tab[i].bs_pid = 1;
		bsm_tab[i].bs_npages = 128;
		}
		j = 0;
		do{ 
			bsm_tab[i].bs_share_id[j] = -1, bsm_tab[i].bs_share_vpno[j] = -1;
			bsm_tab[i].bs_share_npages[j] = 0;
			j++; 
		}while(j < NPROC);
		i++;
	 }
	 restore(ps);
	 return OK;
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
	STATWORD 	ps;
	disable(ps);
	int i = 0;
	do{
		if(bsm_tab[i].bs_status != BSM_UNMAPPED){
			continue;
		}
		else{
	                *avail = i;
                        restore(ps);
                        return OK;
		}
	}while( i < 16);
	restore(ps);
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
	STATWORD 	ps;
	disable(ps);
	bsm_tab[i].bs_share_no = 0;
	if(1){
	bsm_tab[i].bs_npages = 128;
	bsm_tab[i].bs_sem = 1;
	bsm_tab[i].bs_pid = 1;
	}
	bsm_tab[i].bs_vpno = 4096;
	bsm_tab[i].bs_id = i;
	bsm_tab[i].bs_status = BSM_UNMAPPED;
	bsm_tab[i].bs_priv = 0;
	int j = 0;
	while( j < NPROC){
		bsm_tab[i].bs_share_id[j] = -1, bsm_tab[i].bs_share_vpno[j] = -1;
		bsm_tab[i].bs_share_npages[j] = 0; 
		j++;
	}
	int k = 0, l = 0;
	do{
		if(proctab[currpid].bs_id[k] == i){
			break;
		}
		k++;
	} while(k < proctab[currpid].bs_count);
	l = k + 1;
	while(l < proctab[currpid].bs_count){
		proctab[currpid].bs_id[l - 1] = proctab[currpid].bs_id[l];
		proctab[currpid].bs_type[l - 1] = proctab[currpid].bs_type[l];
		proctab[currpid].bs_npages[l - 1] = proctab[currpid].bs_npages[l];
		l++;
	}
	proctab[currpid].bs_count -= 1;
	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
	STATWORD 	ps;
	disable(ps);
	int i = 0;
	while( i < proctab[pid].bs_count){
		int j;
		if(bsm_tab[proctab[pid].bs_id[i]].bs_priv != 1){
			j = 0;
			do{
                                if(bsm_tab[proctab[pid].bs_id[i]].bs_share_id[j] != pid){
					j++;
					continue;
				}
				else{
                                        int value = bsm_tab[proctab[pid].bs_id[i]].bs_share_vpno[j];
                                        int page = ((vaddr >> 12)& 0x000fffff);
					int npages = value + bsm_tab[proctab[pid].bs_id[i]].bs_share_npages[j];
                                        if(value <= page && npages >= page){
                                                *store = proctab[pid].bs_id[i];
                                                *pageth = page - value;
                                                restore(ps);
                                                return OK;
                                        }
                                }
				j++;
                        }while(j < bsm_tab[proctab[pid].bs_id[i]].bs_share_no);
		}
		else{
			int page = ((vaddr >> 12)& 0x000fffff);
			int value = bsm_tab[proctab[pid].bs_id[i]].bs_vpno;
                  
			int npage = value + bsm_tab[proctab[pid].bs_id[i]].bs_npages;
                        if(value <= page && npage >= page){
                                *store = proctab[pid].bs_id[i];
                                *pageth = page - value;
                                restore(ps);
                                return OK;
                        }
		}
		i++;
	}
	restore(ps);
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
	STATWORD 	ps;
	disable(ps);
	if(bsm_tab[source].bs_status != BSM_MAPPED){
		if(1){
		bsm_tab[source].bs_status = BSM_MAPPED;
                bsm_tab[source].bs_pid = pid;
		}
                bsm_tab[source].bs_vpno = vpno;
                bsm_tab[source].bs_npages = npages;
                restore(ps);
                return OK;
	}
	else{
		restore(ps);
		return SYSERR;
	}
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
	STATWORD 	ps;
	disable(ps);
	
	int u = 0, bs_id, page;
	while(u < proctab[pid].bs_count){
		if( bsm_lookup(pid, vpno * 4096, &bs_id, &page) == OK)	break;
		u++;
	}
	
	if(bsm_tab[bs_id].bs_status == BSM_UNMAPPED){
		restore(ps);
		return SYSERR;
	}
	if(bsm_tab[bs_id].bs_priv == 1){
		if(bsm_tab[bs_id].bs_pid != pid){
			restore(ps);
                        return SYSERR;
		}
		else{
			write_back_frames(pid, vpno, bs_id, page);
                        free_bsm(bs_id);
                        restore(ps);
                        return OK;
		}
	}
	else{
		int i = 0, j = 0;
		while(i < bsm_tab[bs_id].bs_share_no){
			if(bsm_tab[bs_id].bs_share_id[i] == pid){
				j = 1;
				break;
			}
			i++;
		}
		if(j){
			if(bsm_tab[bs_id].bs_share_no != 1){
				write_back_frames(pid, vpno, bs_id, page);
                                int k = 0, l =0;
                                do{
                                        if(bsm_tab[bs_id].bs_share_id[k] == pid){
                                                break;
                                        }
					k++;
                                }while(k < bsm_tab[bs_id].bs_share_no);
				l = k + 1;
                                while( l < bsm_tab[bs_id].bs_share_no){
                                        bsm_tab[bs_id].bs_share_id[l - 1] = bsm_tab[bs_id].bs_share_id[l];
                                        bsm_tab[bs_id].bs_share_npages[l - 1] = bsm_tab[bs_id].bs_share_npages[l];
                                        bsm_tab[bs_id].bs_share_vpno[l - 1] = bsm_tab[bs_id].bs_share_vpno[l];
					l++;
                                }
				int max = -1000000;
                                bsm_tab[bs_id].bs_share_no -= 1;
				k = 0;
                                do{
                                        if(max < bsm_tab[bs_id].bs_share_npages[k]){
                                                max = bsm_tab[bs_id].bs_share_npages[k];
                                        }
					k++;
                                } while(k < bsm_tab[bs_id].bs_share_no);
                                
				bsm_tab[bs_id].bs_npages = max;
                        	k = 0;        
				do{
                                        if(proctab[pid].bs_id[k] == bs_id){
                                                break;
                                        }
					k++;
                                }while( k < proctab[pid].bs_count);
				l = k + 1;
                                while(l < proctab[pid].bs_count){
                                        proctab[pid].bs_id[l - 1] = proctab[pid].bs_id[l];
                                        proctab[pid].bs_type[l - 1] = proctab[pid].bs_type[l];
                                        proctab[pid].bs_npages[l - 1] = proctab[pid].bs_npages[l];
					l++;
                                }
				proctab[pid].bs_count -= 1;
                                restore(ps);
                                return OK;
			}
			else{
				write_back_frames(pid, vpno, bs_id, page);
                                free_bsm(bs_id);
                                restore(ps);
                                return OK;
			}
		}
		else{
			restore(ps);
			return SYSERR;
		}
	}
}

void write_back_frames(int pid, int vpno, int bs_id, int page){
	int k = 0, back_store_id, page_id;
	
	while(k < 1024){
		if(frm_tab[k].fr_pid == pid){
			if(frm_tab[k].fr_type == FR_PAGE){
				if(bsm_lookup(pid, frm_tab[k].fr_vpno * 4096, &back_store_id, &page_id) != SYSERR && back_store_id == bs_id){
		
					free_frm(k);
					}
				}
			}
		
		k++;
	}
	k = 0;
	do{
		if(frm_tab[k].fr_pid == pid && frm_tab[k].fr_type == FR_TBL && bsm_tab[bs_id].bs_share_no > 1){
			unsigned long vaddr, pdbr;
			int frame_no_pid;
			unsigned long cr22 = (frm_tab[k].fr_vpno) * 4096;
			virt_addr_t virtual;
			vaddr = frm_tab[k].fr_vpno;
			frame_no_pid = frm_tab[k].fr_pid;
			pdbr = proctab[frame_no_pid].pdbr;

			virtual.pg_offset = ((cr22 & 0x00000fff));
			virtual.pt_offset = ((cr22 & 0x003ff000) >> 12);
			virtual.pd_offset = ((cr22 & 0xffc00000) >> 22);
			
			pd_t *glb_pd;
			pt_t *glb_pt; 
			if(1){
			glb_pd = (pd_t *)(pdbr + (virtual.pd_offset * 4));
			glb_pt = (pt_t *)((glb_pd->pd_base * NBPG) + (virtual.pt_offset * 4));
			}
			if(1){
			frm_tab[k].fr_refcnt = 0;
                        frm_tab[k].fr_count = 0;
			frm_tab[k].fr_pid = -1;
                        }
			frm_tab[k].fr_vpno = -1;
			frm_tab[k].fr_type = FR_NONE;
			frm_tab[k].fr_status = FRM_UNMAPPED;
			delete_frame(k);
			
		}
		k++;
	}while( k < 1024);
}


