#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
	pid_t pid;
	printf("Main process id = %d (parent PID = %d)\n",
			(int) getpid(), (int) getppid());
	
	pid = fork();
	int i;
	int N = 10;

	for (i=0;i<N;i++) printf("[END] Main process id = %d (parent PID = %d)\n",
			(int) getpid(), (int) getppid());
	return 0;
}
