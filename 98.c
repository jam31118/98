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
	
	const char *name ="/OS";
	int shm_fd;
	int *ptr;
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

	printf("Main process id = %d (parent PID = %d)\n", (int)getpid(),(int)getppid());
	*ptr = 0;
	child_pid =  fork();
	int count = 4;
	if (child_pid !=0)
	{
		printf("I AM PARENT\n");
		for(int i=0;i< 10; i++)
		{
			*ptr += 2;
			printf("THIS IS PARENT : shared memory variable is %d\n", *ptr);
		}

	}	
	else
	{
		puts("I AM CHILD");
		for(int i =0; i< 10; i++)
			printf("THIS IS CHILD ; shared memory variable is %d\n", *ptr);
		
	}


	return 0;
}
