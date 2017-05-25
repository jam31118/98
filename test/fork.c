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
	if (pid == 0) {
		for(i=0;i<N;i++) printf("I'm child with pid = %d(with absPID = %d, ppid = %d)\n",pid,(int) getpid(),
				(int) getppid());
	} else {
		for(i=0;i<N;i++) printf("I'm parent with pid = %d (with grandparent ID = %d)\n",
				(int) getpid(), (int) getppid());
	}

	printf("[END] Main process id = %d (parent PID = %d)\n",
			(int) getpid(), (int) getppid());
	return 0;
}
