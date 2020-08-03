#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {
        STATWORD        ps;
        disable(ps);
        
        if(bsm_tab[bs_id].bs_priv != 1){
                int i = 0;
		int j = 0;
                while(i < bsm_tab[bs_id].bs_share_no){
                        if(bsm_tab[bs_id].bs_share_id[i] != currpid){
                                continue;
                        }
			else{
				j = 1;
				break;
			}
			i++;
                }
                if(!j){
                        restore(ps);
                        return SYSERR;
                }
                else{
                        if(bsm_tab[bs_id].bs_share_no != 1){
                                write_back_frame(currpid, bs_id);
                                int k = 0;
				int l = 0;
				do{
                                        if(bsm_tab[bs_id].bs_share_id[k] != currpid)
						continue;
					else
						break;
					k++;
                                }while(k < bsm_tab[bs_id].bs_share_no);
				l = k + 1;
                                while( l < bsm_tab[bs_id].bs_share_no){
                                        bsm_tab[bs_id].bs_share_id[l - 1] = bsm_tab[bs_id].bs_share_id[l];
                                        bsm_tab[bs_id].bs_share_npages[l - 1] = bsm_tab[bs_id].bs_share_npages[l];
                                        bsm_tab[bs_id].bs_share_vpno[l - 1] = bsm_tab[bs_id].bs_share_vpno[l];
					l++;
                                }
                                bsm_tab[bs_id].bs_share_no -= 1;
                                int max = -1000000;
				k = 0;
				do{
                                        if(max >= bsm_tab[bs_id].bs_share_npages[k])
                                               	continue;
					else
						max = bsm_tab[bs_id].bs_share_npages[k];
					k++;
                                }while(k < bsm_tab[bs_id].bs_share_no);

                                bsm_tab[bs_id].bs_npages = max;
                                k = 0;
				while(k < proctab[currpid].bs_count){
                                        if(proctab[currpid].bs_id[k] != bs_id)
                                                continue;
                                        
					else
						break;
					k++;
                                }
				l = k + 1;
				do{
                                        proctab[currpid].bs_id[l - 1] = proctab[currpid].bs_id[l];
                                        proctab[currpid].bs_type[l - 1] = proctab[currpid].bs_type[l];
                                        proctab[currpid].bs_npages[l - 1] = proctab[currpid].bs_npages[l];
					l++;
                                }while(l < proctab[currpid].bs_count);
                                proctab[currpid].bs_count -= 1;
                                restore(ps);
                                return OK;
                        }
                        else{
                                free_bsm(bs_id);
                                restore(ps);
                                return OK;
                        }
                }

        }
        else{
              if(bsm_tab[bs_id].bs_pid != currpid){
                       
                        restore(ps);
                        return SYSERR;
                }
                else{
			write_back_frame(currpid, bs_id);
                        free_bsm(bs_id);
                        restore(ps);
                        return OK;
                }   
        }
}

void write_back_frame(int pid, int bs_id){
	int k = 0, back_store_id, page_id;
	do{
		if(frm_tab[k].fr_pid == pid){
			if(frm_tab[k].fr_type == FR_PAGE){
				int x = bsm_lookup(pid, frm_tab[k].fr_vpno * 4096, &back_store_id, &page_id);
				if(x != SYSERR){
					if(back_store_id != bs_id){
						continue;
					}
					else
					{
						free_frm(k);
					}
				}
			}
		}
		k++;
	} while( k < 1024);
	k = 0;
	while( k < 1024){
		if(frm_tab[k].fr_pid == pid && frm_tab[k].fr_type == FR_TBL && bsm_tab[bs_id].bs_share_no > 1){
			unsigned long vaddr;
			unsigned long pdbr;
			int frame_no_pid;
			pd_t *glb_pd, *glb_pt;
			vaddr = frm_tab[k].fr_vpno;
			frame_no_pid = frm_tab[k].fr_pid;
			pdbr = proctab[frame_no_pid].pdbr;
			unsigned long cr22 = (frm_tab[k].fr_vpno) * 4096;
			virt_addr_t virtual;
			virtual.pg_offset = ((cr22 & 0x00000fff));
			virtual.pt_offset = ((cr22 & 0x003ff000) >> 12);
			virtual.pd_offset = ((cr22 & 0xffc00000) >> 22);
			glb_pd = (pd_t *)(pdbr + (virtual.pd_offset * 4));
			glb_pt = (pt_t *)((glb_pd->pd_base * NBPG) + (virtual.pt_offset * 4));
			frm_tab[k].fr_status = FRM_UNMAPPED;
			frm_tab[k].fr_type = FR_NONE;
                        frm_tab[k].fr_refcnt = 0;
                        frm_tab[k].fr_count = 0;
			frm_tab[k].fr_vpno = -1;
			frm_tab[k].fr_pid = -1;
			delete_frame(k);
		}
		k++;
	}
}

