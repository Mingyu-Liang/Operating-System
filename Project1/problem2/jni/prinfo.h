//struct prinfo
#ifndef _PRINFO_H
#define _PRINFO_H

struct prinfo{
	int parent_pid;
	int pid;
	int first_child_pid;
	int next_sibling_pid;
	long state;
	long uid;
	char comm[64];
};

#endif
