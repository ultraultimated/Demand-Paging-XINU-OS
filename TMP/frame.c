/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <stdio.h>
#include <proc.h>
#include <paging.h>
#include <kernel.h>

/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
 
fr_map_t frm_tab[NFRAMES];
frame_entry frame_tab[NFRAMES];
frame_count = 0;
frame_position = 0;

SYSCALL init_frm()
{
	STATWORD 	ps;
	disable(ps);
	int i = 0;
	do{
		frm_tab[i].fr_count = 0;                                                                                                      frm_tab[i].fr_dirty = 0;
		frm_tab[i].fr_refcnt = 0; 
		frm_tab[i].fr_pid = i;
		frm_tab[i].fr_vpno = -1;
		frm_tab[i].fr_type = FR_NONE;
		frm_tab[i].fr_status = FRM_UNMAPPED;
		i++;
	}while(i < NFRAMES);
	
	int j = 0;
	while( j < NFRAMES){
		frame_tab[j].frame_ref = -1;
		frame_tab[j].frame_frame_no = -1;
		j++;
	}

	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
  STATWORD 	ps;
  disable(ps);
  int i = 0, j = 0;
  while(i < NFRAMES){
		if(frm_tab[i].fr_status == FRM_UNMAPPED){
			*avail = i;
			insert_frame(i);
			break;
		}
		j++;
		i++;
  }
  while(j == 1024){
	  if(grpolicy() == SC){
		  int d = sc_replace();
		  if(d != SYSERR){
			  *avail = d;
                          insert_frame(d);
                          kprintf("\nframe: %d\n",d + 1024);
			  restore(ps);
			  return SYSERR;
		  }
		  else{
			  restore(ps);
			  return SYSERR;
		  }
	  }
	  else if(grpolicy() == LFU){
			int d = lfu_replace();
		  if(d != SYSERR){
	                  *avail = d;
			  insert_frame(d);
			  kprintf("\nframe: %d\n",d + 1024);  
		  }
		  else{
			  restore(ps);
			  return SYSERR;  
		  }
	   }
	j++;
  }
  restore(ps);
  return OK;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{
	STATWORD 	ps;
	disable(ps);
	int frame_no_pid, back_store_id, page_id;
	unsigned long vaddr, pdbr;
	virt_addr_t virtual;
	pd_t *glb_pd;
	pt_t *glb_pt;
	if(frm_tab[i].fr_type == FR_PAGE){
		vaddr = frm_tab[i].fr_vpno;
		unsigned long cr22 = (vaddr) * 4096;
                virt_addr_t virtual; 
		frame_no_pid = frm_tab[i].fr_pid;
		pdbr = proctab[frame_no_pid].pdbr;
		virtual.pg_offset = ((cr22 & 0x00000fff));
		virtual.pt_offset = 0, virtual.pd_offset = 0;
		if(1){
			virtual.pt_offset = ((cr22 & 0x003ff000) >> 12);
			virtual.pd_offset = ((cr22 & 0xffc00000) >> 22);
		}
		int pd_offset = virtual.pd_offset * 4;
		int pt_offset = virtual.pt_offset * 4;
		glb_pd = (pd_t *)(pdbr + pd_offset);
		glb_pt = (pt_t *)((glb_pd->pd_base * NBPG) + pt_offset);
		if(bsm_lookup(frame_no_pid, vaddr * 4096, &back_store_id, &page_id) != SYSERR){
			write_bs((i + 1024) * 4096, back_store_id, page_id);
		}
		int k = 0;
                while(k==0){
		frm_tab[i].fr_type = FR_NONE;
		frm_tab[i].fr_refcnt = 0;
                frm_tab[i].fr_count = 0;
		frm_tab[i].fr_status = FRM_UNMAPPED;
		frm_tab[i].fr_pid = -1;
		frm_tab[i].fr_vpno = -1;
		delete_frame(i);
		glb_pt->pt_base = -1;  
		glb_pt->pt_avail = 0;
		glb_pt->pt_user = 0;
		glb_pt->pt_pcd = 0;
		glb_pt->pt_pwt = 0;
		glb_pt->pt_dirty = 0;
		glb_pt->pt_acc = 0;
		glb_pt->pt_global = 0;
		glb_pt->pt_mbz = 0;
		glb_pt->pt_pres = 0;
		glb_pt->pt_write = 1;
		frm_tab[glb_pd->pd_base - 1024].fr_refcnt -= 1;
		k = 1;
		kprintf("");
		}
		k = 0;
		int baddr = glb_pd->pd_base - 1024;
		if(frm_tab[baddr].fr_refcnt == 0 && k == 0){
			frm_tab[baddr].fr_status = FRM_UNMAPPED;
			frm_tab[baddr].fr_pid = -1;
			frm_tab[baddr].fr_vpno = -1;
			frm_tab[baddr].fr_type = FR_NONE;
			frm_tab[baddr].fr_count = 0;
			delete_frame(baddr);
			int p = 0;
			while(p == 0){
				glb_pd->pd_pres = 0;
				glb_pd->pd_write = 1;
				glb_pd->pd_user = 0;
				glb_pd->pd_pwt = 0;
				glb_pd->pd_pcd = 0;
				if(1){
				glb_pd->pd_acc = 0;
				glb_pd->pd_mbz = 0;
				}
				if(1){
				glb_pd->pd_fmb = 0;
				}
				while(1){
				glb_pd->pd_global = 0;
				glb_pd->pd_avail = 0;
				break;
				}
				glb_pd->pd_base = -1;
				
				p = 1;
			}
			k = 1;
		}
		restore(ps);
		return OK;
	}
	else{
		restore(ps);
		return SYSERR;
	}
}

int sc_replace(){
	while(1){
		if(frm_tab[frame_tab[frame_position].frame_frame_no].fr_type != FR_PAGE){
			
			frame_position++;
                        frame_position = frame_position == frame_count ? 0 : frame_position;
		}
		else{
			unsigned long cr22;
		 	int init = 0;
			if(!init){
				cr22 = (frm_tab[frame_tab[frame_position].frame_frame_no].fr_vpno) * 4096;
			}
                        virt_addr_t virtual;
			int pt_offset = (cr22 & 0x003ff00) >> 12;
			int pd_offset = (cr22 & 0xffc00000) >> 22;
                        virtual.pg_offset = ((cr22 & 0x00000fff));
                        virtual.pt_offset = pt_offset;
                        virtual.pd_offset = pd_offset;
                        pd_t *glb_ptt = (pd_t *)((proctab[frm_tab[frame_tab[frame_position].frame_frame_no].fr_pid].pdbr) + (4 * virtual.pd_offset));
                        if(glb_ptt->pd_pres != 1){
                                continue;                                                                                                             }                                                                                                                             else{                                                                                                                                  pt_t *glb_pt =  (pt_t *)((glb_ptt->pd_base * 4096) + (4 * virtual.pt_offset));                                                if(glb_pt->pt_pres != 1){                                                                                                                                                                                                                                  }                                                                                                                             else{
                                        if(glb_pt->pt_acc != 1){                                                                                                              int frame = frame_tab[frame_position].frame_frame_no;
                                                frame_count = frame_position == frame_count ? 0:frame_count;
                                                free_frm(frame);
                                                return frame;
                                        }
                                        else{
                                                glb_pt->pt_acc = 0;
                                                frame_position++;
                                                frame_position = frame_position == frame_count ? 0 : frame_position;
					}
				}
			}
		}
	}
}

void insert_frame(int i){
	frame_tab[frame_count].frame_frame_no = i;
	frame_tab[frame_count].frame_ref = 1;
	frame_count++;
}

void delete_frame(int value){
	int i = 0;
	int j = 0;
	while(i < frame_count) {
		if(frame_tab[i].frame_frame_no == value){
			break;
		}
		i++;
	}
	if(frame_position > i){
		if(frame_position != 0){
			frame_position -= 1;
		}
		else{
			frame_position = frame_count - 1;
		}
	}
	j = i + 1;
	
	do{
		frame_tab[j - 1].frame_frame_no = frame_tab[j].frame_frame_no;
		frame_tab[j - 1].frame_ref = frame_tab[j].frame_ref;
		j++;
	}while( j < frame_count);
	
	frame_count--;
}

int lfu_replace(){
	int pp = 0;
	int frame_no = -1;
	int max_vpno = -100000;
	int max_count = -100000;
	do{
		int fr_status = frm_tab[pp].fr_status;
		int fr_type = frm_tab[pp].fr_type;
		int fr_count = frm_tab[pp].fr_count;
		int fr_vpno = frm_tab[pp].fr_vpno;
		if(fr_status == FRM_MAPPED){
			if(fr_type == FR_PAGE){
				if(fr_count >= max_count){
					if(fr_vpno >= max_vpno){
						max_count = fr_count;
						max_vpno = fr_vpno;
						frame_no = pp;
					}
				}
			}
		}
		pp+=1;
	}while(pp < 1024);
	return frame_no;
}



