#define _GNU_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

#define PIDMATCH 1

int main () {
	setbuf(stdout,NULL);
	setbuf(stderr,NULL);

	int m = 10;
	pid_t pid_list[m];

	int i;
	for (i=0; i<m; i++) {
		pid_list[i] = fork();
		if (pid_list[i] < 0) {
			perror("fork");
			return 1;
		} else if (pid_list[i] > 0) {
		//	printf("I'm in Parent with pid_list[%d] == %d\n",
		//			i,pid_list[i]);
		} else if (pid_list[i] == 0) {
			printf("I'm in Child with PID == %d, PPID == %d\n",
					(int) getpid(), (int) getppid());
			return 0; // All children dies out here
		}
	}
	
	/* So, the code below are all parents! */
	pid_t tmp, *pid_p, *pid_p_max = pid_list + m;
	int status = -1;
	
	for(pid_p=pid_list; pid_p<pid_p_max; ++pid_p) {
	//	tmp = waitpid(*pid_p, &status, PIDMATCH);
		tmp = wait(&status);
		printf("Child with PID=%ld exited with status %d\n",(long)tmp, status);
	}
	
	
}

