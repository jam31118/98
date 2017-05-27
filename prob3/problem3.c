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

#define BUFSIZE 20

typedef struct sh_data {
	int buffer[BUFSIZE];
	int SUM;
	int *write_ptr, *read_ptr;
} sh_data_t;

int producer(sh_data_t *sh_data_p, sem_t *empty, sem_t *mutex, sem_t *full) {
	return 0;
}

int consumer(sh_data_t *sh_data_p, sem_t *empty, sem_t *mutex, sem_t *full) {
	return 0;
}

int main(int argc, char *argv[]) {
	/* Parsing */
	if (argc <= 1) {printf("Enter m, n values\n");return 1;}
	int m = atoi(argv[1]);
	int n = atoi(argv[2]);

	pid_t producer_pids[m], consumer_pids[n];
	int totalProcNum = m + n;
	int procAliveNum = totalProcNum;

	/* Global configuration */
	//int bufSize = 20;
	//int n = 2; // number of processes

	/* Shared memory information */
	const char *shmName = "/SHM";
	size_t SIZE = sizeof(sh_data_t);
	sh_data_t *sh_data_p;
	int shm_fd;
	
	/* Shared memory declaration */
	shm_fd = shm_open(shmName, O_CREAT | O_RDWR, 0666);
	if (shm_fd == -1) {printf("Shared failed in CREATing\n"); return 1;}
	if (ftruncate(shm_fd,SIZE)) printf("[ERROR] Failed to ftruncate()\n");
	sh_data_p = (sh_data_t *) mmap(0,SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,shm_fd,0);
	if(sh_data_p == MAP_FAILED) {printf("Map Failed\n");return 1;}

	/* Shared Data initialization */
	sh_data_p->SUM = 0;
	int i; for (i=0; i<BUFSIZE; i++) *(sh_data_p->buffer + i) = 0;
	sh_data_p->write_ptr = sh_data_p->buffer;
	sh_data_p->read_ptr = sh_data_p->buffer;
		
	/* Semaphore information */
	const char *fullName = "/FULL";
	const char *mutexName = "/MUTEX";
	const char *emptyName = "/EMPTY";
	
	/* Semaphore ID generation */
	sem_t *full_id = sem_open(fullName, O_CREAT, S_IRUSR | S_IWUSR, 1);
	sem_t *empty_id = sem_open(emptyName, O_CREAT, S_IRUSR | S_IWUSR, 1);
	sem_t *mutex_id = sem_open(mutexName, O_CREAT, S_IRUSR | S_IWUSR, 1);

	/* Creating Child Processes */
	pid_t ch1, ch2;

	ch1 = fork();
	if (ch1) {
		/* Parent Process */
		ch2 = fork();
		if (ch2) {
			/* Parent Process in deeper but same */
			while (procAliveNum>0) {wait(NULL); --procAliveNum;}
			printf("%d\n",sh_data_p->SUM);
		} else {
			/* Child Processes: Consumers */
			for ( i = 0; i < n; ++i ) {
				consumer_pids[i] = fork();
				if (consumer_pids[i] < 0) {
					fprintf(stderr,"[ERROR] Failed to fork()\n");
					return 1;
				} else if (consumer_pids[i] == 0) {
					consumer(sh_data_p,empty_id,mutex_id,full_id);
				}
			}
		}

	} else {
		/* Child Processes: Producers */
		for ( i = 0; i < m; ++i ) {
			producer_pids[i] = fork();
			if (producer_pids[i] < 0) {
				fprintf(stderr,"[ERROR] Failed to fork()\n");
				return 1;
			} else if (producer_pids[i] == 0) {
				producer(sh_data_p,empty_id,mutex_id,full_id);
			}
		}
	}
	return 0;
}
