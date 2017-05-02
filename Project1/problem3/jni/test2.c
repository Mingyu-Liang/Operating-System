/*
* Operating System Project1 Problem3
* Copyright (c) 2017 Marsl
*/
//------------------------------------------------------------
/*
Generate a new process and output “StudentIDParent” with PID, then generates its children process output “StudentIDChild” with PID.
Use execl to execute ptree in the child process,show the relationship between above two process.
*/
//-----------------------------------------------------------
#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include<stdlib.h>
#include<string.h>
int main(int argc,char*argv[])
{	
	pid_t pid;
	pid = fork();
	if(pid == 0) //children process
	{
		printf("StudentNumberChild PID:%d\n",getpid());
		execl("/data/misc/testARM","testARM",(char*)NULL);
		_exit(EXIT_SUCCESS);
	}
	else if(pid >0) //parent process
	{	
		waitpid(pid,NULL,0);
		printf("StudentNumberParent PID:%d\n",getpid());
	}
	else //fork error!
	{	
		printf("Fork error!\n");
		exit(EXIT_FAILURE);
	}
	wait(10);
	return 0;
}
