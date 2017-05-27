#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#define TRUE 1
#define FALSE 0
int isStringDouble(char *s);
int main(int argc, char * argv[]) {
	pid_t child_pid;
	int status;
	const char *name ="/OS";
	int shm_fd;
	int *ptr;
	int register1, register2;
	double slp;
	//const int SIZE 4096;
	//argv 
	if (argc == 1)
	{
		printf("Worng argument, enter sleep duration\n");
		exit(1);
	}
	else
	{
		if (isStringDouble(argv[1]))
			slp = atof(argv[1]);
		else
			puts("argument is not number");
	}

	// start declaring shared memory
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



	// make two child process 
//	printf("Main process id = %d (parent PID = %d)\n", (int)getpid(),(int)getppid());
	*ptr = 0;
	// make first child 
	child_pid =  fork();

	if (child_pid !=0)
	{
		//make second child 
		child_pid = fork();
		// parent waiting for child to finish and show finial value of shared memeory
		if (child_pid != 0)
		{
			wait(&status);
			printf("Shared variable : %d\n",*ptr);
		}
		// second child : adding shared memory 
		else
		{
			for(int i=0;i< 10; i++)
			{
				register1 = *ptr;
				printf("Register1 Value : %d\n",register1);
				//usleep(slp);

				register1++;
				printf("Register1 Value plused : %d\n",register1);
				//usleep(slp);

				*ptr = register1;
				printf("Shared memory value : %d\n",*ptr);
				//usleep(slp);

			}
		}
	}	
	// first child : substracting shared memory
	else
	{
		for(int i =0; i< 10; i++)
		{
			register2 = *ptr;
			printf("Register2 Value : %d\n",register2);
			//usleep(slp);

			register2--;
			printf("Register2 Value minused : %d\n",register2);

			//usleep(slp);

			*ptr = register2;
			printf("Shared memory value : %d\n",*ptr);

			//usleep(slp);

		}
	}


	return 0;
}

int isStringDouble(char *s) {
	  size_t size = strlen(s);
	    if (size == 0) return 0; // 0바이트 문자열은 숫자가 아님

		  for (int i = 0; i < (int) size; i++) {
			      if (s[i] == '.' || s[i] == '-' || s[i] == '+') continue;
				      if (s[i] < '0' || s[i] > '9') return 0; // 알파벳 등이 있으면 숫자 아님
					    }

		    return 1; // 그밖의 경우는 숫자임
}
