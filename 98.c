#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/shm.h>
#define TRUE 1
#define FALSE 0

int main() {
	pid_t child_pid;
	// shared memory 
	int status;
	const char *name ="/OS";
	int shm_fd;
	int *ptr;
	int register1;
	int register2;
	//const int SIZE 4096;
	shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
	if (shm_fd == -1){
		printf("Shared memory segment failed\n");
		exit(-1);
	}
	ftruncate(shm_fd, sizeof(int));
	ptr = mmap(0,sizeof(int) ,PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd,0);
	if (ptr == MAP_FAILED){
		printf("Map failed\n");
		return -1;
	}
	//
	printf("Main process id = %d (parent PID = %d)\n", (int)getpid(),(int)getppid());
	*ptr = 0;
	child_pid =  fork();
	//int count = 4;
	if (child_pid !=0)
	{
		child_pid = fork();
		if (child_pid != 0)
		{
			wait(&status);
			printf("Shared variable : %d\n",*ptr);
		}
		else{
	//	printf("I AM PARENT\n");
		fflush(stdout);
		for(int i=0;i< 10; i++)
		{
			register1 = *ptr;
		//	printf("REISTER1 : %d\n",register1);
			usleep(10);
			fflush(stdout);
			register1++;
		//	printf("REISTER1 : %d\n",register1);
			usleep(10);
			fflush(stdout);
			*ptr = register1;
		//	printf("THIS IS PARENT : shared memory variable is %d\n", *ptr);
			usleep(10);
			fflush(stdout);
		}
	}
	}	
	else
	{
	//	puts("I AM CHILD");
		fflush(stdout);
		for(int i =0; i< 10; i++)
		{
			register2 = *ptr;
	//		printf("REISTER2 : %d\n",register2);
			usleep(10);
			fflush(stdout);
			register2--;
	//		printf("REISTER2 : %d\n",register2);
			usleep(10);
			fflush(stdout);
			*ptr = register2;
	//		printf("THIS IS CHILD ; shared memory variable is %d\n", *ptr);
			usleep(10);
			fflush(stdout);
		}
	}


	return 0;
}
