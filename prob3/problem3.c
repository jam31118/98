#define _GNU_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
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
	int write_idx, read_idx;
	int consumed, m;
} sh_data_t;

int producer(sh_data_t *sh_data_p ,sem_t *empty, sem_t *mutex, sem_t *full){
	int num; // idx;
	//idx =0;
	num = 1;
	do{
		sem_wait(empty);
			
		sem_wait(mutex);
		*sh_data_p->write_ptr = num++;
		sh_data_p->write_ptr = (sh_data_p->buffer+((sh_data_p->write_idx++)%20));
		sem_post(mutex);
		sem_post(full);
		
	} while (num<=50);
	return 0;
}

int consumer(sh_data_t *sh_data_p, sem_t *empty, sem_t *mutex , sem_t *full) {
	//int idx; idx = 0;
	do{
		sem_wait(full);
		sem_wait(mutex);
		
		sh_data_p->SUM	+=	*sh_data_p->read_ptr;
		*sh_data_p->read_ptr = 0;
		sh_data_p->read_ptr = (sh_data_p->buffer+((sh_data_p->read_idx++)%20));
		sh_data_p->consumed++;	

		sem_post(mutex);
		sem_post(empty);
	}while(sh_data_p->consumed <= (sh_data_p->m)*50);

	return 0;	
}
/*
int producer(sh_data_t *sh_data_p, sem_t *empty, sem_t *mutex, sem_t *full) {
	return 0;
}

int consumer(sh_data_t *sh_data_p, sem_t *empty, sem_t *mutex, sem_t *full) {
	return 0;
}*/

int main(int argc, char *argv[]) {
	/* Parsing */
	if (argc <= 1) {printf("Enter m, n values\n");return 1;}
	int m = atoi(argv[1]);
	int n = atoi(argv[2]);

	int totalProcNum = m + n;
	int procAliveNum = totalProcNum;
	pid_t producer_pids[m], consumer_pids[n], wait_result_pids[totalProcNum];

	/* Shared memory information */
	const char *shmName = "/SHM";
	size_t SIZE = sizeof(sh_data_t);
	sh_data_t *sh_data_p;
	int shm_fd;
	
	/* Shared memory declaration */
	shm_unlink(shmName);
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
	sh_data_p->write_idx = 0;
	sh_data_p->read_idx = 0;
	sh_data_p->consumed =0;
	sh_data_p->m = m;
	/* Semaphore information */
	const char *fullName = "/FULL";
	const char *mutexName = "/MUTEX";
	const char *emptyName = "/EMPTY";
	
	/* Semaphore ID generation */
	sem_t *full_id = sem_open(fullName, O_CREAT, S_IRUSR | S_IWUSR, 1);
	sem_t *empty_id = sem_open(emptyName, O_CREAT, S_IRUSR | S_IWUSR, BUFSIZE);
	sem_t *mutex_id = sem_open(mutexName, O_CREAT, S_IRUSR | S_IWUSR, 0);

	/* Creating Child Processes */
	pid_t ch1, ch2;

	ch1 = fork();
	int status=0;
	if (ch1) {
		/* Parent Process */
		//while(procAliveNum>0)
		//{
		//	wait(&status);
		//	--procAliveNum;
		//}
//		printf("%d\n",sh_data_p->SUM);
		ch2 = fork();
		if (ch2) {
			/* Parent Process in deeper but same */
			while (procAliveNum>0) {
				wait(&status); 
				--procAliveNum;
			}
			printf("%d\n",sh_data_p->SUM);
		} else {
			/* Child Processes: Consumers */
			for ( i = 0; i < n; ++i ) {
				consumer_pids[i] = fork();
				if (consumer_pids[i] < 0) {
					fprintf(stderr,"[ERROR] Failed to fork()\n");
					return 1;
				} else if (consumer_pids[i] == 0) {
			//		fprintf(stderr,"[ LOG ] I'm in Comsumers PID == %d, SUM == %d\n",
			//				(int) getpid(),sh_data_p->SUM);
					consumer(sh_data_p,empty_id,mutex_id,full_id);
					return 0;
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
		//		fprintf(stderr,"[ LOG ] I'm in Producers PID == %d, SUM == %d\n",
		//				(int) getpid(), sh_data_p->SUM);
				producer(sh_data_p,empty_id,mutex_id,full_id);
				return 0;
			}
		}
	}
	return 0;
}
