/*
	VATranslate.c
	function: translate the virtual address to physical address in the target process
	command line: ./VATranslate PID Virtural address(belonging to this process)
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
    // check error
    if(err){
        printf("Expose page table error!");
        return;
    }

    unsigned long Vir_pgd = begin_vaddr >> 21;
    unsigned long Vir_pte = (begin_vaddr >> PAGE_SHIFT) & 0x1FF;
    unsigned long Vir_offset = begin_vaddr & 0xFFF;

    // translate virtual address to physical address
    unsigned long *ptr = (unsigned long*)((unsigned long*)fake_pgd)[Vir_pgd];
    unsigned long PageFrame = ptr[Vir_pte] & (~0xFFF);
    unsigned long Physical_addr = PageFrame + Vir_offset;
    // show the result
    printf("Virtual Address to Physical Address Translation:\n");
    printf("Virtual Address: 0x%08lx\n", begin_vaddr);
    printf("Pgd: 0x%03lx\tPte: 0x%03lx\tOffset: 0x%03lx\n", Vir_pgd, Vir_pte, Vir_offset);
    printf("Ptr: 0x%08lx\n", (unsigned long)ptr);
    printf("PageFrame: 0x%08lx\n", PageFrame);
    printf("Physical Address: 0x%08lx\n", Physical_addr);
}

int main(int argc, char *argv[]){
	if(argc != 3){
		printf("Wrong Parameters!\n");
		printf("Usage:./VATranslate Pid VA\n");
		return -1;
	}
	// get pagetable layout
	Get_Pagetable_Layout();
	// read parameters
	pid_t pid = atoi(argv[1]);
	unsigned long begin_vaddr = strtoul(argv[2], NULL, 0);
    unsigned long end_vaddr = begin_vaddr + 1;
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
