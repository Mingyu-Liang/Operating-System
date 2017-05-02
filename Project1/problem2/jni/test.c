/*
* Operating System Project1 problem2
* Copyright (c) 2017 Marsl
*/
//-----------------------------------------------------------
/*
*Test your new system call
*a simple C program which calls ptree
*the entire process tree (in DFS order)
*using tabs to indent children with respect to their parents.
*/
//-----------------------------------------------------------
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<string.h>
#include"prinfo.h"
int stack[100];
int id = -1;
int j;
int length=0;
struct prinfo *pf;

int main()
{
	pf = malloc(100*sizeof(struct prinfo));
	printf("Begin\n\n");
	syscall(356,pf,&length);
        printf("Number of processes: %d\n",length);
	for(j=0;j<length;j++)
	{
		if(id == -1)
		{
			stack[++id]=pf[j].pid;
		}
		else
		{
			while(id != -1 && pf[j].parent_pid != stack[id])
			{
				id--;
			}
			if(id == -1)
			{
				stack[++id]=pf[j].pid;
			}
			else
			{
				int i;
				for(i=0;i<=id;i++) 
					printf("\t");
				stack[++id]=pf[j].pid;
			}	
		}
		printf("%s,%d,%ld,%d,%d,%d,%d\n",pf[j].comm,pf[j].pid,pf[j].state,pf[j].parent_pid,pf[j].first_child_pid,pf[j].next_sibling_pid,pf[j].uid);
	}
	printf("End\n");
	free(pf);
	return 0;
}
