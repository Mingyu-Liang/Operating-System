/*
	vm_inspector.c
	function: dump the page table entries of a process in given range
	command line: ./vm_inspector  pid  begin_addresss end_address
*/
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>

#define __NR_get_pagetable_layout 356
#define __NR_expose_page_table 357

#define MY_PGD_SIZE 2048

struct pagetable_layout_info{
    uint32_t pgdir_shift;
    uint32_t pmd_shift;
    uint32_t page_shift;
};

void Get_Pagetable_Layout(){
	struct pagetable_layout_info pgtbl_info;
	// call syscall "get_pagetable_layout"
	int err = syscall(__NR_get_pagetable_layout, &pgtbl_info, sizeof(struct pagetable_layout_info));
	if(err){
		printf("Get pagetable layout error!");
		return;
	}
	printf("pgdir_shift: %u\n", pgtbl_info.pgdir_shift);
	printf("pmd_shift: %u\n", pgtbl_info.pmd_shift);
	printf("page_shift: %u\n\n", pgtbl_info.page_shift);
}

void Expose_Page_Table(pid_t pid, unsigned long fake_pgd, unsigned long page_table_addr, unsigned long begin_vaddr, unsigned long end_vaddr){
    // call syscall "expose_page_table"
    int err = syscall(__NR_expose_page_table, pid, fake_pgd, page_table_addr, begin_vaddr, end_vaddr);
    if(err){
        printf("Expose page table error!");
        return;
    }
    unsigned long Vir_pgd;
    unsigned long Vir_pte;
    unsigned long Vir_offset;
    unsigned long *ptr;
    unsigned long PageFrame;
    unsigned long Physical_addr;
    unsigned long current_va;
    current_va = begin_vaddr;

    printf("Virtual Address to Physical Address Translation:\n");

    while(current_va <= end_vaddr)
    {
    	Vir_pgd = current_va >> 21;
    	Vir_pte = (current_va >> PAGE_SHIFT) & 0x1FF;
    	Vir_offset = current_va & 0xFFF;
    	// translate virtual address to physical address
    	*ptr = (unsigned long*)((unsigned long*)fake_pgd)[Vir_pgd];
    	PageFrame = ptr[Vir_pte] & (~0xFFF);
    	Physical_addr = PageFrame + Vir_offset;
    	// show the result
		if (PageFrame)
		{
			printf("\tVirtual Address: 0x%08lx\n", current_va);
			printf("\tPhysical Address: 0x%08lx\n", Physical_addr);
		}
    	current_va += 4096;
    }
}

int main(int argc, char *argv[]){
	if(argc != 4){
		printf("Wrong Parameters!\n");
		printf("Usage:./vm_inspector  pid  begin_addresss end_address \n");
		return -1;
	}
	//get the parameters
	pid_t pid = atoi(argv[1]);
	unsigned long begin_vaddr = strtoul(argv[2], NULL, 0);
    unsigned long end_vaddr = strtoul(argv[3], NULL, 0);
    // allocate space and check error
    unsigned long fake_pgd = (unsigned long)malloc(MY_PGD_SIZE * PAGE_SIZE);
    unsigned long page_table_addr = (unsigned long)mmap(NULL, PAGE_SIZE * PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if(!fake_pgd || !page_table_addr){
		printf("No enough free space!\n");
		return -1;
	}
	// expose pagetable
	Expose_Page_Table(pid, fake_pgd, page_table_addr, begin_vaddr, end_vaddr);
	// free allocated space
	free((void*)fake_pgd);
    munmap((void*)page_table_addr, PAGE_SIZE * PAGE_SIZE);
	
	return 0;
}
