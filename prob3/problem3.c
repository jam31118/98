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

typedef struct sh_data{
	int buffer[BUFSIZE];
	int SUM;
	int *write_ptr, *read_ptr;
	int write_idx, read_idx;
	int consumed, m_value;
} sh_data_t;

//semaphore "mutex" is binary semaphore
//semaphore "empty" and "full" is counting semaphore
int producer(sh_data_t *sh_data_p ,sem_t *empty, sem_t *mutex, sem_t *full){
	printf("producer process is begin \n");
	int num = 0; // idx;
	int emp_val;	//to know empty semaphore value
	//idx =0;
	do{
<<<<<<< HEAD
		printf("producer process's wait function will be executed \n");
		sem_wait(empty);	//check that there is any empty buffer
		sem_getvalue(&empty, &emp_val);	//get empty semaphore value
		printf("empty value is %d \n", emp_val);
		sem_wait(mutex);	//check that this process can be run (if another producer is running or consumer is running, then this process will be blocked)
		
		//producing an item is performed
		num = num + 1;
		*(sh_data_p->write_ptr) = num;
		printf("item %d is produced \n", num);
		sh_data_p->write_ptr = (sh_data_p->buffer + ((sh_data_p->write_idx++)%20));
		
		sem_post(mutex);	//adding item is done. wake up another process which is in the waiting queue (by block())
		sem_post(full);
		
		fprintf(stderr,"[ LOG ] BUFFER in Producer(PID==%d) ",(int) getpid());
		printBuffer(sh_data_p->buffer);
	} while (num<=50);
	return 0;
}

int consumer(sh_data_t *sh_data_p, sem_t *empty, sem_t *mutex , sem_t *full) {
	printf("consumer process is begin \n");
	do{
<<<<<<< HEAD
		printf("consumer process's wait function will be executed \n");
		sem_wait(full);		//check that there is any full buffer. if it has only empty buffer, then block().
		sem_wait(mutex);	//check that this process can be run
=======
		fprintf(stderr,"[ LOG ] (before sema-full) I'm in Consumer(PID==%d)\n",(int) getpid());
		sem_wait(full);
		fprintf(stderr,"[ LOG ] (after sema-full) I'm in Producer(PID==%d)\n",(int) getpid());
		sem_wait(mutex);
>>>>>>> 1e7d9a246cba2627e2e662b7fc6bf118bb827b5a
		
		//consuming an item is performed
		sh_data_p->SUM	= sh_data_p->SUM + *(sh_data_p->read_ptr);
		printf("item is added in SUM. Current SUM = %d \n", sh_data_p->SUM);
		sh_data_p->read_ptr = 0;
		sh_data_p->read_ptr = (sh_data_p->buffer + ((sh_data_p->read_idx++)%20));
		sh_data_p->consumed++;

		sem_post(mutex);
<<<<<<< HEAD
		sem_post(empty);
	}while(sh_data_p->consumed <= (sh_data_p->m_value)*50);
=======
		sem_post(full);
>>>>>>> 1e7d9a246cba2627e2e662b7fc6bf118bb827b5a

		fprintf(stderr,"[ LOG ] BUFFER in Producer(PID==%d) ",(int) getpid());
		printBuffer(sh_data_p->buffer);
	}while(1);
	
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
	if (argc <= 1) 
	{
		printf("Enter m, n values\n");
		return 1;
	}
	int m = atoi(argv[1]);
	int n = atoi(argv[2]);

//	int totalProcNum = m + n;
	//int procAliveNum = totalProcNum;
	pid_t producer_pids[m], consumer_pids[n];
//	pid_t wait_result_pids[totalProcNum];

	/* Shared memory information */
	const char *shmName = "/SHM";
	size_t SIZE = sizeof(sh_data_t);
	sh_data_t* sh_data_p;
	int shm_fd;
	
	/* Shared memory declaration */
	shm_fd = shm_open(shmName, O_CREAT | O_RDWR, 0666);
	if (shm_fd == -1) 
	{
		printf("Shared failed in CREATing\n");
		return 1;
	}
	if (ftruncate(shm_fd,SIZE)) 
	{
		printf("[ERROR] Failed to ftruncate()\n");
	}
	sh_data_p = (sh_data_t *) mmap(0,SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,shm_fd,0);
	if(sh_data_p == MAP_FAILED) 
	{
		printf("Map Failed\n");
		return 1;
	}

	/* Shared Data initialization */
	sh_data_p->SUM = 0;
	int i; for (i=0; i<BUFSIZE; i++) *(sh_data_p->buffer + i) = 0;
	sh_data_p->write_ptr = sh_data_p->buffer;
	sh_data_p->read_ptr = sh_data_p->buffer;
	sh_data_p->write_idx = 0;
	sh_data_p->read_idx = 0;
	sh_data_p->consumed =0;
	sh_data_p->m_value = m;

	/* Semaphore information */
	const char *fullName = "/FULL";
	const char *mutexName = "/MUTEX";
	const char *emptyName = "/EMPTY";
	
	/* Semaphore ID generation */
	sem_t *full_id = sem_open(fullName, O_CREAT, S_IRUSR | S_IWUSR, 0);
	sem_t *empty_id = sem_open(emptyName, O_CREAT, S_IRUSR | S_IWUSR, BUFSIZE);
	sem_t *mutex_id = sem_open(mutexName, O_CREAT, S_IRUSR | S_IWUSR, 1);

	/* Creating Child Processes */
	pid_t ch1, ch2;

	ch1 = fork();
<<<<<<< HEAD
	int status=0;
	if (ch1) 
	{
=======
	int status = -1;
	pid_t tmp;
	if (ch1) {
>>>>>>> 1e7d9a246cba2627e2e662b7fc6bf118bb827b5a
		/* Parent Process */
		ch2 = fork();
		if (ch2) 
		{
			/* Parent Process in deeper but same */
<<<<<<< HEAD
			while (procAliveNum>0) 
			{
				wait(&status); 
=======
			/*
			while (procAliveNum>0) {
				wait_result_pids[totalProcNum - procAliveNum] = wait(&status);
>>>>>>> 1e7d9a246cba2627e2e662b7fc6bf118bb827b5a
				--procAliveNum;
				fprintf(stderr,"[ LOG ] Child(PID==%d) exited with status 0x%x\n",
						wait_result_pids[totalProcNum - procAliveNum], status);
			}*/
			for (i=0; i<m; i++) {
				tmp = waitpid(producer_pids[i],&status,0);
				fprintf(stderr,"[ LOG ] Producers(PID==%d) exited with status 0x%x\n",
						tmp, status);	
				fflush(stderr);
			}
			for (i=0; i<n; i++) {
				tmp = waitpid(consumer_pids[i],&status,0);
				fprintf(stderr,"[ LOG ] Consumers(PID==%d) exited with status 0x%x\n",
						tmp, status);	
				fflush(stderr);
			}
			printf("%d \n", sh_data_p->SUM);
			printf("%d \n", status);

		} 
		else 
		{
			/* Child Processes: Consumers */
			for ( i = 0; i < n; ++i ) 
			{
				consumer_pids[i] = fork();
				if (consumer_pids[i] < 0) {
					fprintf(stderr,"[ERROR] Failed to fork()\n");
					return 1;
<<<<<<< HEAD
				} 
				else if (consumer_pids[i] == 0) 
				{
					fprintf(stderr,"[ LOG ] I'm in Comsumers PID == %d, SUM == %d\n", (int) getpid(),sh_data_p->SUM);
=======
				} else if (consumer_pids[i] == 0) {
					fprintf(stderr,"[ LOG ] I'm in Comsumers PID==%d, PPID==%d, SUM == %d\n",
							(int) getpid(), (int) getppid(), sh_data_p->SUM);
					fflush(stderr);
>>>>>>> 1e7d9a246cba2627e2e662b7fc6bf118bb827b5a
					consumer(sh_data_p,empty_id,mutex_id,full_id);
					return 0;
				}
			}
		}

	} 
	else 
	{
		/* Child Processes: Producers */
		for ( i = 0; i < m; ++i ) 
		{
			producer_pids[i] = fork();
			if (producer_pids[i] < 0) 
			{
				fprintf(stderr,"[ERROR] Failed to fork()\n");
				return 1;
<<<<<<< HEAD
			}
			else if (producer_pids[i] == 0) 
			{
				fprintf(stderr,"[ LOG ] I'm in Producers PID == %d, SUM == %d\n", (int) getpid(), sh_data_p->SUM);
=======
			} else if (producer_pids[i] == 0) {
				fprintf(stderr,"[ LOG ] I'm in Producers PID==%d, PPID==%d, SUM == %d\n",
						(int) getpid(), (int) getppid(), sh_data_p->SUM);
				fflush(stderr);
>>>>>>> 1e7d9a246cba2627e2e662b7fc6bf118bb827b5a
				producer(sh_data_p,empty_id,mutex_id,full_id);
				return 0;
			}
		}
	}
	return 0;
}
