#define _GNU_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main () {
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
			return 0;
		} else {
			printf("Semething got run\n");
		}
	}
}

