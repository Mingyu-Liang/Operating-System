/*
* Operating system Project1 problem1 
   515010910101-liangmingyu
*/
//------------------------------------------------------------
/*
*Write a new system call in Linux.
*The system call you write should take two arguments
*and return the process tree information in a depth-
*first-search (DFS) order.
*Each system call must be assigned a number. Your
*system call should be assigned number 356.
*/
//-----------------------------------------------------------
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#include <linux/unistd.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/pid.h>
#include <linux/init_task.h>
#include <linux/spinlock.h>
#include <linux/rwlock_types.h>
#include <linux/rwlock.h>
#include"prinfo.h"
MODULE_LICENSE("Dual BSD/GPL");
#define __NR_pstree 356
DEFINE_RWLOCK(buf_lock);
/*--------------------------------
*copy information from kernel to kbuffer
---------------------------------*/
void copy_ptree(struct task_struct *task, struct prinfo *kbuffer, int *id)
{
	kbuffer[*id].pid = task->pid;
	kbuffer[*id].uid = task->cred->uid;
	kbuffer[*id].state = task->state;
	get_task_comm(kbuffer[*id].comm, task);
	kbuffer[*id].parent_pid = (task->real_parent == NULL) ? 0 : task->real_parent->pid;
	kbuffer[*id].first_child_pid = (list_empty(&task->children) == 1) ? 0 : (list_entry(task->children.next, struct task_struct, sibling)->pid);
	kbuffer[(*id)++].next_sibling_pid = (list_empty(&task->sibling) == 1) ? 0 : (list_entry(task->sibling.next, struct task_struct, sibling)->pid);
	printk("The current pid is %d\n", task->pid);
}
/*--------------------------------
*the DFS function
---------------------------------*/
void _dfs_ptree(struct task_struct *cur, struct prinfo *kbuffer, int *id)
{ 
	if (cur == NULL)
		return;
	copy_ptree(cur, kbuffer, id);
	struct task_struct *tmp;
	if (list_empty(&cur->children) != 1)
	{
		list_for_each_entry(tmp, &cur->children, sibling)
		{
			_dfs_ptree(tmp, kbuffer, id);
		}
	}
	return;
}
static int(*oldcall)(void);
/*--------------------------------
* build the pstree and copy to user interface
---------------------------------*/
int ptree_build(struct prinfo __user *buf, int __user *nr)
{
	int __kernel knr = 0;
	struct prinfo __kernel *kbuffer;
	kbuffer = kmalloc(100 * sizeof(struct prinfo), GFP_KERNEL);
	if (buf == NULL || nr == NULL)
		return 0;
	read_lock(&tasklist_lock);
	_dfs_ptree(&init_task, kbuffer, &knr);
	read_unlock(&tasklist_lock);
	copy_to_user(nr, &knr, sizeof(int));
	printk("Number of processes : %d\n", knr);
	if (copy_to_user(buf, kbuffer, knr * sizeof(struct prinfo)))
		printk("Error at copy to user !\n");
	kfree(kbuffer);
	return 0;
}
static int addsyscall_init(void)
{
	long *syscall = (long*)0xc000d8c4;
	oldcall = (int(*)(void))(syscall[__NR_pstree]);
	syscall[__NR_pstree] = (unsigned long)ptree_build;
	printk(KERN_INFO "module load!\n");
	return 0;
}

static void addsyscall_exit(void)
{
	long *syscall = (long*)0xc000d8c4;
	syscall[__NR_pstree] = (unsigned long)oldcall;
	printk(KERN_INFO "module exit!\n");
}

module_init(addsyscall_init);
module_exit(addsyscall_exit);

