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

#include <errno.h>
#include <string.h>

#define BUFSIZE 20
#define MAXNUM 50

#define SHMNAME "/SHM"
#define FULLNAME "/FULL"
#define MUTEXNAME "/MUTEX"
#define EMPTYNAME "/EMPTY"

typedef struct sh_data {
	int buffer[BUFSIZE];
	int SUM;
	int *write_ptr, *read_ptr;
	int write_idx, read_idx;
	int m, n;
	int consumer_end_flag;
} sh_data_t;

int isEmpty(int *buffer, size_t len) {
    int *buf_p, *buf_p_max = buffer + len;
    for(buf_p=buffer; buf_p<buf_p_max; buf_p++){
        if (*buf_p) { return 0; }
    }
    return 1;
}

/* Producer function */
int producer() {
	/* Open Shared memory */
	const size_t SIZE = sizeof(sh_data_t);
	int shm_fd = shm_open(SHMNAME, O_RDWR, 0666);
	if (shm_fd == -1) {fprintf(stderr,"[PID=%d] Shared failed in CREATing (errno == %s\n",(int)getpid(),strerror(errno)); return 1;}
	sh_data_t *sh_data_p = (sh_data_t *) 
		mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if(sh_data_p == MAP_FAILED) {fprintf(stderr,"[PID=%d] Map Failed\n",(int)getpid());return 1;}

	/* Open Semaphores */
	sem_t *empty, *mutex, *full;
	empty = sem_open(EMPTYNAME,0);
	mutex = sem_open(MUTEXNAME,0);
	full = sem_open(FULLNAME,0);
	
	/* Data initialization */
	int num = 0;
	int in_idx = 0;

	/* Starting Iteration for Data Production */
	do{

        /* Wait for Consumers to consume the data */
		if (sem_wait(empty)) {
			fprintf(stderr,"[PID=%d] Failed to sem_wait(empty)\n",(int)getpid()); 
			return 1; }
        /* Wait for other processes to go out */
		if (sem_wait(mutex)) {
			fprintf(stderr,"[PID=%d] Failed to sem_wait(mutex)\n",(int)getpid());
			return 1; }
        
        /* Produce data */
        num++;

        /* Calculate the writing index(in_idx) of buffer */
		in_idx = sh_data_p->write_idx % BUFSIZE;
        
        /* Put the produced data into buffer using writing index(in_idx) */
		sh_data_p->buffer[in_idx] = num;
        
        /* Increment the number of numbers that have written */
		sh_data_p->write_idx++;
		
        /* Inform other processes the unlocked mutex by posting 'mutex' */
		if (sem_post(mutex)) {
			fprintf(stderr,"[PID==%d] Failed to Producer sem_post(mutex)\n",(int) getpid());
			return 1; }
        /* Inform other process the incremented number of elements in buffer by posting 'full' */
		if (sem_post(full)) {
			fprintf(stderr,"[PID==%d] Failed to Producer sem_post(full)\n",(int) getpid());
			return 1; }
		
		/* Break Condition */
		if (num >= MAXNUM) break;

	} while (1);

	/* Close Shared memory */
	close(shm_fd);

	/* Close Semaphores */
	sem_close(empty);
	sem_close(mutex);
	sem_close(full);

	return 0;
}

/* Consumer function */
int consumer() {
	/* Open Shared memory */
	const size_t SIZE = sizeof(sh_data_t);
	int shm_fd = shm_open(SHMNAME, O_RDWR, 0666);
	if (shm_fd == -1) {fprintf(stderr,"[PID=%d] Shared failed in CREATing (errno == %s\n",(int)getpid(),strerror(errno)); return 1;}
	sh_data_t *sh_data_p = (sh_data_t *) 
		mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if(sh_data_p == MAP_FAILED) {fprintf(stderr,"[PID=%d] Map Failed\n",(int)getpid());return 1;}

	/* Open Semaphores */
	sem_t *empty, *mutex, *full;
	empty = sem_open(EMPTYNAME,0);
	mutex = sem_open(MUTEXNAME,0);
	full = sem_open(FULLNAME,0);

	/* Data Initialization */
	int out_idx = 0;
	int fullWriteIdx, fullReadIdx, bufferIsEmpty;
	/* All three variables should be set true to terminate consumers */
		
	/* Start Interation for Data Consumption */
	do{
        /* Wait for Procuders to produce new data */
		if (sem_wait(full)) {
            fprintf(stderr,"[PID=%d] Failed to sem_wait(full)\n",(int)getpid());
            return 1; }
        /* Wait for other processes to go out */
		if (sem_wait(mutex)) {
            fprintf(stderr,"[PID=%d] Failed to sem_wait(mutex)\n",(int)getpid());
            return 1; }
		
		if (sh_data_p->consumer_end_flag == 0) {

	        /* Determine index to read from buffer */
			out_idx = sh_data_p->read_idx % BUFSIZE;
	        
	        /* Read and use the data */
	        sh_data_p->SUM += sh_data_p->buffer[out_idx];
	
	        /* Clearing the buffer section that has already been read */
			sh_data_p->buffer[out_idx] = 0;
	
	        /* Increment the reading index to next element of buffer */
			sh_data_p->read_idx += 1; // 170529 Problem! (170529 solved)
		} 

		/* Determining Break Conditions */
		fullWriteIdx = sh_data_p->write_idx >= (sh_data_p->m * MAXNUM);
		fullReadIdx = sh_data_p->read_idx >= (sh_data_p->m * MAXNUM); // m is right. not n.
		bufferIsEmpty = isEmpty(sh_data_p->buffer, BUFSIZE);
		sh_data_p->consumer_end_flag = fullWriteIdx && fullReadIdx && bufferIsEmpty;

        /* Inform other processes the unlocked mutex by posting 'mutex' */
		if (sem_post(mutex)) {
            fprintf(stderr,"[PID==%d] Failed to Consumer sem_post(mutex)\n",(int) getpid());
            return 1; }
        /* Inform other process the decremented number of elements in buffer by posting 'empty' */
		if (sem_post(empty)) {
            fprintf(stderr,"[PID==%d] Failed to Consumer sem_post(empty)\n",(int) getpid());
            return 1; }
		
		if (sh_data_p->consumer_end_flag) {
			int i;
			for(i=0; i<sh_data_p->n-1; i++) {
				// to close all other n-1 consumers
				if (sem_post(full)) {
		            fprintf(stderr,"[PID==%d] Failed to Consumer sem_post(full)\n",(int) getpid());
        		    return 1; }
			}
			break;
		}

	} while (1);
	
	/* Close Shared memory */
	close(shm_fd);

	/* Close Semaphores */
	sem_close(empty);
	sem_close(mutex);
	sem_close(full);

	return 0;
}

int main(int argc, char *argv[]) {
	/* Parsing (m for producer, n for consumer)*/
	if (argc <= 1) {printf("Enter m, n values\n");return 1;}
	int m = atoi(argv[1]);
	int n = atoi(argv[2]);
    
    /* Declaring pid container for multiple processes */ 
	pid_t producer_pids[m], consumer_pids[n];

    /* Disabling buffering in standard output and error 
     * so the printf and fprintf yield its content immediately */
	setbuf(stderr,NULL);
	setbuf(stdout,NULL);

	/* Shared memory information */
	const char *shmName = "/SHM";
	size_t SIZE = sizeof(sh_data_t);
	sh_data_t *sh_data_p;
	int shm_fd;
	
	/* Shared memory declaration */
	shm_fd = shm_open(shmName, O_CREAT | O_RDWR, 0666);
	if (shm_fd == -1) {fprintf(stderr,"[PID=%d] Shared failed in CREATing (errno == %s\n",(int)getpid(),strerror(errno)); return 1;}
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
	sh_data_p->m = m;
	sh_data_p->n = n;
	sh_data_p->consumer_end_flag = 0;
		
	/* Semaphore ID generation */
	sem_t *full_id = sem_open(FULLNAME, O_CREAT, S_IRUSR | S_IWUSR, 0);
	sem_t *empty_id = sem_open(EMPTYNAME, O_CREAT, S_IRUSR | S_IWUSR, BUFSIZE);
	if (empty_id == SEM_FAILED) {fprintf(stderr,"empty_id is failed\n");}
	sem_t *mutex_id = sem_open(MUTEXNAME, O_CREAT, S_IRUSR | S_IWUSR, 1);
    
    /* Semaphore Initialization */
	sem_init(full_id,1,0);
	sem_init(empty_id,1,BUFSIZE);
	sem_init(mutex_id,1,1);

    
	/* Creating Child Processes */
	pid_t *pid_p;
	pid_t *prod_pid_p_max = producer_pids + m;
	pid_t *consum_pid_p_max = consumer_pids + n;
	
    /* Producer Process */
	for (pid_p=producer_pids; pid_p<prod_pid_p_max; pid_p++) {
		*pid_p = fork();
		if (*pid_p < 0) {
			fprintf(stderr,"[ERROR] Failed to fork()\n");
			return 1;
		} else if (*pid_p == 0) {
			producer();
			return 0;
		}
	}

    /* Consumer Process */
	for (pid_p=consumer_pids; pid_p<consum_pid_p_max; pid_p++) {
		*pid_p = fork();
		if (*pid_p < 0) {
			fprintf(stderr,"[ERROR] Failed to fork()\n");
			return 1;
		} else if (*pid_p == 0) {
			consumer();
			return 0;
		}
	}
	
    /* Wait for Child processes */
	int status = -1;
	int totalProcessNum = m + n;
	for (i=0; i<totalProcessNum; i++)
	    wait(&status);
   

	/* Unlink Shared memory */
	shm_unlink(SHMNAME);

	/* Close and Unlink Semaphores */
	sem_close(empty_id);
	sem_unlink(EMPTYNAME);
	sem_close(mutex_id);
	sem_unlink(MUTEXNAME);
	sem_close(full_id);
	sem_unlink(FULLNAME);
            
    /* Printing final summation result */
	printf("%d\n",sh_data_p->SUM);

}
