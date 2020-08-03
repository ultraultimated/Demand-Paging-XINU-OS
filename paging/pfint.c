/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>


/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{
	STATWORD 	ps;
	disable(ps);
	unsigned long cr2 = read_cr2();
	virt_addr_t virtual;
	int bs_store;
	int bs_page;
	int tmp = bsm_lookup(currpid, cr2, &bs_store, &bs_page);
	unsigned long pdbr_value = proctab[currpid].pdbr;
	if(tmp != SYSERR){
	int back = shared_frame(bs_store, bs_page, cr2);
	virtual.pd_offset = ((cr2 & 0xffc00000) >> 22);
	virtual.pt_offset = ((cr2 & 0x003ff000) >> 12);
	virtual.pg_offset = ((cr2 & 0x00000fff));

	int free_frame;
	pt_t *glb_pt;
	pd_t *glb_ptt = (pd_t *)(pdbr_value + (4 * virtual.pd_offset)); 
	
		if(glb_ptt->pd_pres){
		frm_tab[glb_ptt->pd_base - 1024].fr_refcnt += 1;
		if(back > 0){
			free_frame = back;
                        frm_tab[free_frame].fr_refcnt += 1;
		}
		else{
			int k = 0;
			while(k == 0){
			k = 1;
			get_frm(&free_frame);

			frm_tab[free_frame].fr_refcnt = frm_tab[free_frame].fr_refcnt < 0 ? 0 : frm_tab[free_frame].fr_refcnt;
                        frm_tab[free_frame].fr_type = FR_PAGE;
			frm_tab[free_frame].fr_status = FRM_MAPPED;
			frm_tab[free_frame].fr_vpno = ((cr2 >> 12)& 0x000fffff);
                        frm_tab[free_frame].fr_pid = currpid;
 frm_tab[free_frame].fr_count = 1;
                        frm_tab[free_frame].fr_refcnt += 1;
                        read_bs((1024 + free_frame) * 4096, bs_store, bs_page);
			
			}
		}
		int init = 0;
		glb_pt = (pt_t *)((glb_ptt->pd_base * 4096) + (4 * virtual.pt_offset));
		if(!init){
		glb_pt->pt_write = 1;
                glb_pt->pt_pres = 1;
		}
		while(1){
		glb_pt->pt_avail = 0;
                glb_pt->pt_global = 0; 
		glb_pt->pt_user = 0;
		break;
		}
		glb_pt->pt_mbz = 0;
                glb_pt->pt_acc = 0;
		int pt_base = free_frame + 1024;
		glb_pt->pt_pcd = 0;
		glb_pt->pt_pwt = 0;
		glb_pt->pt_dirty = 0;
		glb_pt->pt_base = pt_base;
		
	}
	else{
		int pt_frame_no;
		get_frm(&pt_frame_no);
		        frm_tab[pt_frame_no].fr_refcnt = 0;
		while(1){	
		frm_tab[pt_frame_no].fr_pid = currpid;
			frm_tab[pt_frame_no].fr_status = FRM_MAPPED;
                        frm_tab[pt_frame_no].fr_vpno = 1024 + pt_frame_no;
                        frm_tab[pt_frame_no].fr_type = FR_TBL;
                	break;
		}        
		frm_tab[pt_frame_no].fr_count = 1;
                        frm_tab[pt_frame_no].fr_refcnt += 1;
		glb_pt = (pt_t *)((1024 + pt_frame_no) * NBPG);
		
		int k =0;
		do{
			while(1){
			glb_pt->pt_user = 0;
			glb_pt->pt_pwt = 0;
			glb_pt->pt_pres = 0;
			glb_pt->pt_write = 0;
			glb_pt->pt_acc = 0;
			glb_pt->pt_pcd = 0;
			glb_pt->pt_dirty = 0;
			glb_pt->pt_mbz = 0;
			glb_pt->pt_global = 0;
			glb_pt->pt_avail = 0;
			glb_pt->pt_base = 0;
			glb_pt++;
			k++;
			break;
			}
		}while(k < 1024);
		int init = 0;
		while(!init){

		glb_ptt->pd_pwt = 0;
                glb_ptt->pd_pcd = 0;
                glb_ptt->pd_acc = 0;
		init = 1;
		}
		glb_ptt->pd_pres = 1;
		glb_ptt->pd_write = 1;
		glb_ptt->pd_user = 0;
		glb_ptt->pd_mbz = 0;
		glb_ptt->pd_fmb = 0;
		glb_ptt->pd_global = 0;
		glb_ptt->pd_avail = 0;
		glb_ptt->pd_base = 1024 + pt_frame_no;
		if(back < 0){
			get_frm(&free_frame);
			frm_tab[free_frame].fr_vpno = ((cr2 >> 12)& 0x000fffff);
                        frm_tab[free_frame].fr_type = FR_PAGE;
			frm_tab[free_frame].fr_status = FRM_MAPPED;
			frm_tab[free_frame].fr_pid = currpid;
                        frm_tab[free_frame].fr_refcnt = frm_tab[free_frame].fr_refcnt < 0 ? 0 : frm_tab[free_frame].fr_refcnt;
                        frm_tab[free_frame].fr_count = 1;
                        frm_tab[free_frame].fr_refcnt += 1;
			read_bs((1024 + free_frame) * 4096, bs_store, bs_page);
		}
		else{
			frm_tab[back].fr_refcnt += 1;
		}
		glb_ptt = (pd_t *)(pdbr_value + (4 * virtual.pd_offset));
		glb_pt = (pt_t *)(((1024 + pt_frame_no) * 4096) + (4 * virtual.pt_offset));
		while(1){
		glb_pt->pt_pres = 1;
                glb_pt->pt_write = 1;
                glb_pt->pt_user = 0;
                glb_pt->pt_pwt = 0;
                glb_pt->pt_pcd = 0;
                do{
		glb_pt->pt_acc = 0;
                glb_pt->pt_mbz = 0;
                glb_pt->pt_dirty = 0;
                glb_pt->pt_global = 0;
                glb_pt->pt_avail = 0;
		glb_pt->pt_base = free_frame + 1024;
		break;
		}while(1);
		break;
		}
	}
	restore(ps);
	return OK;

	}
	else{
		kill(currpid);
                restore(ps);
                return SYSERR;
	}

}

int shared_frame(int bs_store, int bs_page, unsigned long cr2){
	virt_addr_t virtual;
	if(bsm_tab[bs_store].bs_priv != 1 && bsm_tab[bs_store].bs_share_no > 1 && (cr2 > 0x10000000)){
			int i = 0;
			while(i < bsm_tab[bs_store].bs_share_no){
				unsigned long cr22 = (bsm_tab[bs_store].bs_share_vpno[i] + (bs_page)) * 4096;
				if(bsm_tab[bs_store].bs_share_id[i] != currpid && bsm_tab[bs_store].bs_share_npages[i] >= bs_page){
					
					virtual.pg_offset = ((cr22 & 0x00000fff));
					virtual.pd_offset = ((cr22 & 0xffc00000) >> 22);
					virtual.pt_offset = ((cr22 & 0x003ff000) >> 12);
					pd_t *glb_ptt = (pd_t *) ((proctab[bsm_tab[bs_store].bs_share_id[i]].pdbr) + (4 * virtual.pd_offset));
					if(glb_ptt->pd_pres){
						pt_t *glb_pt = 	(pt_t *)((glb_ptt->pd_base * 4096) + (4 * virtual.pt_offset));
						if(glb_pt->pt_pres){
							return (glb_pt->pt_base - 1024);
						}
						
					}
					
				}
				i++;
			}
	}
	else{
		return -1;
	}
	return -1;
}


