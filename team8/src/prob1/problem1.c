#define _GNU_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>

#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>

/* Child Processes1 */
int ch1task(int *X, int N, int sleeptime) {
	usleep(sleeptime);
	int i;
	for(i=0;i<N;i++) (*X)++; // = *sh_data_p +1;
	return 0;
}
/* Child Processes2 */
int ch2task(int *X, int N, int sleeptime) {
	usleep(sleeptime);
	int i;
	for(i=0; i<N; i++) (*X)--; // = *sh_data_p -1;
	return 0;
}

int main(int argc, char *argv[]) {
	/* Parsing */
	int sleeptime1;
    int sleeptime2;
    if (argc <= 1) {sleeptime1 = 0;sleeptime2 = 0;}
    else{
    sleeptime1 = atoi(argv[1]);
	sleeptime2 = atoi(argv[2]);
    }
	/* Global configuration */
	int N = 10;
	int n = 2; // number of processes

	/* Shared memory information */
	const char *shmName = "/SHM";
	size_t SIZE = sizeof(int);
	int *X;
	int shm_fd;

    /* Creating Shared memory */
	shm_fd = shm_open(shmName, O_CREAT | O_RDWR, 0666);
	if (shm_fd == -1) {printf("Shared failed in CREATing\n"); return 1;}
	if (ftruncate(shm_fd,SIZE)) printf("[ERROR] Failed to ftruncate()\n");
	X = (int *) mmap(0,SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,shm_fd,0);
	if(X == MAP_FAILED) {printf("Map Failed\n");return 1;}

	/* Shared Data initialization */
	*X = 0;

	/* Creating Child Processes */
	pid_t ch1, ch2;

	ch1 = fork();
	if (ch1) {
		/* Parent Process */
		ch2 = fork();
		if (ch2) {
			/* Parent Process in deeper but same */
			while (n>0) {wait(NULL); --n;}
			printf("%d\n",*X);
		} else {
			/* Child Process 02 */
			ch2task(X,N,sleeptime1);
		}

	} else {
		/* Child Process 01 */
		ch1task(X,N,sleeptime2);
	}
	return 0;
}
