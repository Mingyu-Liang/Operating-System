/*
	pagetable_syscall.c
	This program implements two system calls: get_pagetable_layout and expose_page_table
	Syscall get_pagetable_layout£º
		ID: 356
		function: Investigate the page table layout 
	Syscall expose_page_table£º
		ID: 357
		function: Map a target process's page table into current process with a virtual memory range
*/
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/uaccess.h>
#include <linux/mm_types.h>
#include <asm/errno.h>
#include <asm/pgtable.h>

#define __NR_get_pagetable_layout 356
#define __NR_expose_page_table 357

#define MY_PGD_SIZE 2048

MODULE_LICENSE("Dual BSD/GPL");

struct pagetable_layout_info{
    uint32_t pgdir_shift;
    uint32_t pmd_shift;
    uint32_t page_shift;
};

int get_pagetable_layout(struct pagetable_layout_info __user * pgtbl_info, int size){
    struct pagetable_layout_info layout;
    if(sizeof(struct pagetable_layout_info) > size){
        printk(KERN_INFO "No enough memory!\n");
        return -EINVAL;
    }
    layout.pgdir_shift = PGDIR_SHIFT;
    layout.pmd_shift = PMD_SHIFT;
    layout.page_shift = PAGE_SHIFT;
    if(copy_to_user(pgtbl_info, &layout, sizeof(struct pagetable_layout_info))){
        printk(KERN_INFO "Pagetable layout copy to user failed!\n");       
        return -EFAULT;
    }
    return 0;
}

struct walk_info{
    unsigned long *fake_pgd; 
    unsigned long usr_page_table_addr; // user space point
};

int copy_pmd_entry(pmd_t* pmd, unsigned long addr, unsigned long next, struct mm_walk* walk)
{
    unsigned long pmdIndex = pgd_index(addr);
    struct walk_info* my_walk_info = (struct walk_info*)walk->private;
    struct vm_area_struct* vma = find_vma(current->mm, my_walk_info->usr_page_table_addr);
    unsigned long pfn;
    int err;
    if(!pmd){
        printk(KERN_INFO "None pmd!\n");
        return 0;
    }
    if(!vma){
        printk(KERN_INFO "Vma error!\n");
        return -EFAULT;
    }   
    if(pmd_bad(*pmd)){
        printk(KERN_INFO "Bad pmd!\n");
        return -EINVAL;
    }
    pfn = page_to_pfn(pmd_page((unsigned long)*pmd));
    if(!pfn_valid(pfn)){
        printk(KERN_INFO "Pfn invalid!\n");
        return -EFAULT;
    }
    // remap target pagetable entries to user space
    err = remap_pfn_range(vma, my_walk_info->usr_page_table_addr, pfn, PAGE_SIZE, vma->vm_page_prot);
    if(err){
        printk(KERN_INFO "Remap_pfn_range failed!\n");
        return err;
    }
    // update fake pgd and pte information
    my_walk_info->fake_pgd[pmdIndex] = my_walk_info->usr_page_table_addr;
    my_walk_info->usr_page_table_addr += PAGE_SIZE;
    return 0;
}

int expose_page_table(pid_t pid, unsigned long fake_pgd, unsigned long page_table_addr, unsigned long begin_vaddr, unsigned long end_vaddr){
    // check parameters
    if(!(begin_vaddr < end_vaddr) || !fake_pgd || !page_table_addr){
        printk(KERN_INFO "Input parameters wrong!\n");
        return -EINVAL;
    }
    struct pid *pid_struct = find_get_pid(pid);
    if(!pid_struct){
    	printk(KERN_INFO "No such pid!\n");
        return -EINVAL;
    }
    struct task_struct* target_tsk = get_pid_task(pid_struct, PIDTYPE_PID);
    if(!target_tsk){
        printk(KERN_INFO "No such pid_struct!\n");        
        return -EINVAL;
    }
    struct walk_info my_walk_info;
    my_walk_info.fake_pgd = (pgd_t*)kcalloc(MY_PGD_SIZE, sizeof(pgd_t), GFP_KERNEL);
    if(!my_walk_info.fake_pgd){
        printk(KERN_INFO "No enough free space!\n");      
        return -EFAULT;
    }
    my_walk_info.usr_page_table_addr = page_table_addr;

    struct mm_walk walk;
    walk.pgd_entry = NULL;
    walk.pud_entry = NULL;
    walk.pmd_entry = copy_pmd_entry;
    walk.pte_entry = NULL;
    walk.pte_hole = NULL;
    walk.hugetlb_entry = NULL;
    walk.mm = target_tsk->mm;
    walk.private = (void*)(&my_walk_info);

    // walk through virtual memory address
    down_write(&target_tsk->mm->mmap_sem); // lock
    walk_page_range(begin_vaddr, end_vaddr, &walk);
    up_write(&target_tsk->mm->mmap_sem); // unlock

    if(copy_to_user((void*) fake_pgd, my_walk_info.fake_pgd, MY_PGD_SIZE * sizeof(pgd_t))){
        kfree(my_walk_info.fake_pgd);
        printk(KERN_INFO "Copy to user error!\n");       
        return -EFAULT;
    }
    kfree(my_walk_info.fake_pgd);
    return 0;
}

static int (*oldcall_NR_get_pagetable_layout)(void), (*oldcall_NR_expose_page_table)(void);

static int pagetable_syscall_init(void){
    long *syscall = (long*)0xc000d8c4;
    oldcall_NR_get_pagetable_layout = (int(*)(void))(syscall[__NR_get_pagetable_layout]); 
    syscall[__NR_get_pagetable_layout] = (unsigned long)get_pagetable_layout; 
    printk(KERN_INFO "Module Get_pagetable_layout load.\n");
    oldcall_NR_expose_page_table = (int(*)(void))(syscall[__NR_expose_page_table]); 
    syscall[__NR_expose_page_table] = (unsigned long)expose_page_table; 
    printk(KERN_INFO "Module Expose_page_table load.\n");
    return 0;
}

static void pagetable_syscall_exit(void){
    long *syscall = (long*)0xc000d8c4;
    syscall[__NR_get_pagetable_layout] = (unsigned long)oldcall_NR_get_pagetable_layout; 
    printk(KERN_INFO "Module Get_Pagetable_Layout exit.\n");
    syscall[__NR_expose_page_table] = (unsigned long)oldcall_NR_expose_page_table; 
    printk(KERN_INFO "Module Expose_Page_Table exit.\n");
}

module_init(pagetable_syscall_init);
module_exit(pagetable_syscall_exit);
