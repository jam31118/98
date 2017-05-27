#define _GNU_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>

#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <semaphore.h>

typedef struct sh_data {
	int buffer[20];
	int SUM;
	int *write_ptr, *read_ptr;
} sh_data_t;

int ch1task(int *sh_data_p, int N, int sleeptime) {
	usleep(sleeptime);
	int i;
	for(i=0;i<N;i++) (*sh_data_p)++; // = *sh_data_p +1;
	return 0;
}

int ch2task(int *sh_data_p, int N, int sleeptime) {
	usleep(sleeptime);
	int i;
	for(i=0; i<N; i++) (*sh_data_p)--; // = *sh_data_p -1;
	return 0;
}

int main(int argc, char *argv[]) {
	/* Parsing */
	if (argc <= 1) {printf("Enter m, n values\n");return 1;}
	int m = atoi(argv[1]);
	int n = atoi(argv[2]);

	/* Global configuration */
	//int N = 10;
	//int n = 2; // number of processes

	/* Shared memory information */
	const char *shmName = "/SHM";
	size_t SIZE = sizeof(sh_data_t);
	int *sh_data_p;
	int shm_fd;
	
	/* Shared memory declaration */
	shm_fd = shm_open(shmName, O_CREAT | O_RDWR, 0666);
	if (shm_fd == -1) {printf("Shared failed in CREATing\n"); return 1;}
	if (ftruncate(shm_fd,SIZE)) printf("[ERROR] Failed to ftruncate()\n");
	sh_data_p = (sh_data_t *) mmap(0,SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,shm_fd,0);
	if(sh_data_p == MAP_FAILED) {printf("Map Failed\n");return 1;}

	/* Shared Data initialization */
	*sh_data_p;
		
	/* Semaphore information */
	const char *semName = "/SEM";
	
	/* Semaphore ID generation */
	sem_t *sem_id = sem_open(semName, O_CREAT, S_IRUSR | S_IWUSR, 1);

	/* Creating Child Processes */
	pid_t ch1, ch2;

	ch1 = fork();
	if (ch1) {
		/* Parent Process */
	//	printf("I'm PARENT (PID == %d)\n",
	//			(int) getpid());
		//wait(NULL);
		ch2 = fork();
		if (ch2) {
			/* Parent Process in deeper but same */
		//	printf("[PID == %d] I'm PARENT, in inner shell \n",
		//			(int) getpid());
			while (n>0) {wait(NULL); --n;}
			printf("%d\n",*sh_data_p);
		} else {
			/* Child Process 02 */
		//	printf("[PID == %d] I'm CHILD \n",
		//			(int) getpid());
			sem_wait(sem_id);
			ch2task(sh_data_p,N,sleeptime1);
			sem_post(sem_id);
		}

	} else {
		/* Child Process 01 */
	//	printf("[PID == %d] I'm CHILD\n",
	//			(int) getpid());
		sem_wait(sem_id);
		ch1task(sh_data_p,N,sleeptime2);
		sem_post(sem_id);
	}
	return 0;
}
