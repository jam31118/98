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
	/*
	const char *name ="/OS";
	int shm_fd;
	void *ptr;
	const int SIZE 4096;
	shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
	ftruncate(shm_fd, SIZE);
	*/

	printf("Main process id = %d (parent PID = %d)\n", (int)getpid(),(int)getppid());
	child_pid =  fork();
	int count = 4;
	if (child_pid !=0)
	{
		printf("Parent: child's process id = %d\n", child_pid);
		printf("Parent Count: %d\n",count++);
		printf("Parent Count: %d\n",count)i;
	}	
	else
	{
		printf("Child : my process id = %d\n",(int) getpid());
		printf("Child Count: %d\n",count);
	}
	return 0;
}
