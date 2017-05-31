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

void printBuffer(int *buffer) {
	int *buf_p;
	fprintf(stdout,"[PID=%d] [[",(int) getpid());
	for(buf_p = buffer; buf_p<buffer+BUFSIZE; buf_p++) {
		fprintf(stdout,"%3d",*buf_p);
	}
	fprintf(stdout,"]]");
	fprintf(stdout,"\n");
}

int isEmpty(int *buffer, size_t len) {
    int *buf_p, *buf_p_max = buffer + len;
    for(buf_p=buffer; buf_p<buf_p_max; buf_p++){
        if (*buf_p) { return 0; }
    }
    return 1;
}

void printSEM(sem_t *mutex, sem_t *full, sem_t *empty) {
	int mutex_val, full_val, empty_val;
	if (sem_getvalue(mutex,&mutex_val)) {
		fprintf(stderr,"[ERROR] [PID==%d] Failed to get sem value (errno == %s)\n",(int)getpid(),strerror(errno));	}
	if (sem_getvalue(full,&full_val)) {
		fprintf(stderr,"[ERROR] [PID==%d] Failed to get sem value (errno == %s)\n",(int)getpid(),strerror(errno));	}
	if (sem_getvalue(empty,&empty_val)) {
		fprintf(stderr,"[ERROR] [PID==%d] Failed to get sem value (errno == %s)\n",(int)getpid(),strerror(errno));	}
	fprintf(stdout,"[PID=%d] mutex=%d, full=%d, empty=%d\n",
			(int) getpid(),mutex_val, full_val, empty_val);
}

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
        
        /* Debug info: division line between processes */
		fprintf(stdout,"\n=======PRODUCER-MUTEX-START======[SUM=%d][PID=%d]\n",sh_data_p->SUM, (int) getpid());
        
        /* Produce data */
        num++;

        /* Calculate the writing index(in_idx) of buffer */
		in_idx = sh_data_p->write_idx % BUFSIZE;
        
        /* Put the produced data into buffer using writing index(in_idx) */
		sh_data_p->buffer[in_idx] = num;
        
        /* Debug info */ 
		fprintf(stdout,"[PID=%d] sh_data_p->write_idx == %d\n",(int)getpid(),sh_data_p->write_idx);
		fprintf(stdout,"[PID=%d] in_idx == %d\n",(int)getpid(),in_idx);
		printBuffer(sh_data_p->buffer);
        
        /* Increment the number of numbers that have written */
		sh_data_p->write_idx++;
		
        /* Debug info */ 
		printSEM(mutex,full,empty);
        
        /* Debug info: division line between processes */
		fprintf(stdout,"=======PRODUCER-MUTEX-ENDS=======[SUM=%d][PID=%d]\n",sh_data_p->SUM, (int) getpid());

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

	/* Comment on unlinking Shared memory
     * One should not unlink for sake of other processes
     * Since shm_unlink() destroies the shared memory object permanently! */
	
	/* Close Shared memory */
	close(shm_fd);

	/* Close Semaphores */
	sem_close(empty);
	sem_close(mutex);
	sem_close(full);

	return 0;
}

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
		
        /* Debug info: division line between processes */
		fprintf(stdout,"\n+++++CONSUMER-MUTEX-START+++++++[SUM=%d][PID=%d]\n",
				sh_data_p->SUM, (int) getpid());
        
        /* Debug info */
		printSEM(mutex,full,empty);
       	
		if (sh_data_p->consumer_end_flag == 0) {

	        /* Determine index to read from buffer */
			out_idx = sh_data_p->read_idx % BUFSIZE;
	        
	        /* Read and use the data */
	        sh_data_p->SUM += sh_data_p->buffer[out_idx];
	
	        /* Clearing the buffer section that has already been read */
			sh_data_p->buffer[out_idx] = 0;
	
	        /* Debug info */
			fprintf(stdout,"[PID=%d] read_idx = %d\n",(int)getpid(),sh_data_p->read_idx);
			fprintf(stdout,"[PID=%d] out_idx == %d\n",(int)getpid(),out_idx);
			printBuffer(sh_data_p->buffer);
	        
	        /* Increment the reading index to next element of buffer */
			sh_data_p->read_idx += 1; // 170529 Problem! (170529 solved)
		} else {
			fprintf(stdout,"[PID=%d] Since the task is completed, there's nothing to do in critical section. I'm terminating . . .\n",(int)getpid());
		}

        /* Debug info: division line between processes */
		fprintf(stdout,"+++++CONSUMER-MUTEX-ENDS++++++++[SUM=%d][PID=%d]\n\n",
				sh_data_p->SUM, (int) getpid());

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
		
		/* Break Condition */
	//	if ((sh_data_p->write_idx >= MAXNUM) && isEmpty(sh_data_p->buffer,BUFSIZE)) break; // 170529-1517 Problem .. should read all (170531 solved!) // 170531-2303 problem.. wrong for mmnn case (170531-2316 solved)
//		if (fullWriteIdx && fullReadIdx && bufferIsEmpty) break; // 170531-2317 problem. when breaking, all other consumers (and possibly also producers) is still waiting for ul.
//		if (sh_data_p->consumer_end_flag) break;
		
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
	
	/* Comment on unlinking Shared memory
     * One should not unlink for sake of other processes
     * Since shm_unlink() destroies the shared memory object permanently! */

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

	/* Debug info: checking initialization */
	printSEM(mutex_id,full_id,empty_id);

	
	pid_t *pid_p;
	pid_t *prod_pid_p_max = producer_pids + m;
	pid_t *consum_pid_p_max = consumer_pids + n;
	
	for (pid_p=producer_pids; pid_p<prod_pid_p_max; pid_p++) {
		*pid_p = fork();
		if (*pid_p < 0) {
			fprintf(stderr,"[ERROR] Failed to fork()\n");
			return 1;
		} else if (*pid_p == 0) {
			fprintf(stderr,"[ LOG ] I'm in Producers PID==%d, PPID==%d\n",(int) getpid(), (int) getppid());
			producer();
			return 0;
		}
	}

	for (pid_p=consumer_pids; pid_p<consum_pid_p_max; pid_p++) {
		*pid_p = fork();
		if (*pid_p < 0) {
			fprintf(stderr,"[ERROR] Failed to fork()\n");
			return 1;
		} else if (*pid_p == 0) {
			fprintf(stderr,"[ LOG ] I'm in Consumers PID==%d, PPID==%d\n",(int) getpid(), (int) getppid());
			consumer();
			return 0;
		}
	}
	
	pid_t tmp;
	int status = -1;
	int totalProcessNum = m + n;
	for (i=0; i<totalProcessNum; i++) {
		tmp = wait(&status);
		fprintf(stdout,"[PID=%d] Process(PID==%d) exited with status %d\n",(int) getpid(), tmp, status);
	}

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


	/* Creating Child Processes */
	/*
	pid_t ch1, ch2;
	ch1 = fork();
	int status = -1;
	pid_t tmp;
	*/
//	if (ch1) {
//		/* Parent Process */
//		ch2 = fork();
//		if (ch2) {
//            /* Wait for child processes to terminate and get their exit statuses */
//            /*
//			tmp = wait(&status);
//			fprintf(stderr,"[PID=%d] Producers(PID==%d) exited with status %d\n",(int) getpid(), tmp, status);
//			tmp = wait(&status);
//			fprintf(stderr,"[PID=%d] Consumers(PID==%d) exited with status %d\n",(int) getpid(), tmp, status);	
//            */
//			/*
//			for (i=0; i<m; i++) {
//				tmp = waitpid(producer_pids[i],&status,0);
//				fprintf(stderr,"[ LOG ] [PID==%d] Producer exited with status 0x%x\n",tmp, status);	
//			}
//			for (i=0; i<n; i++) {
//				tmp = waitpid(consumer_pids[i],&status,0);
//				fprintf(stderr,"[ LOG ] [PID==%d] Consumer exited with status 0x%x\n",tmp, status);	
//			}*/
//			/* 170531 NOTE 
//			 * When using waitpid with option > 0, the mother process 
//			 * terminate faster than children */
//
//			int totalProcessNum = m + n;
//			for (i=0; i<totalProcessNum; i++) {
//				tmp = wait(&status);
//				fprintf(stdout,"[PID=%d] Process(PID==%d) exited with status %d\n",(int) getpid(), tmp, status);
//			}
//
//			/* Unlink Shared memory */
//			shm_unlink(SHMNAME);
//		
//			/* Close and Unlink Semaphores */
//			sem_close(empty_id);
//			sem_unlink(EMPTYNAME);
//			sem_close(mutex_id);
//			sem_unlink(MUTEXNAME);
//			sem_close(full_id);
//			sem_unlink(FULLNAME);
//            
//            /* Printing final summation result */
//			printf("%d\n",sh_data_p->SUM);
//
//		} else {
//            /* Child Process: Consumer */
//			//consumer();
//			/* Child Processes: Consumers */
//			for ( i = 0; i < n; ++i ) {
//				consumer_pids[i] = fork();
//				if (consumer_pids[i] < 0) {
//					fprintf(stderr,"[ERROR] Failed to fork()\n");
//					return 1;
//				} else if (consumer_pids[i] == 0) {
//					fprintf(stderr,"[ LOG ] I'm in Comsumers PID==%d, PPID==%d\n",(int) getpid(), (int) getppid());
//					//consumer(sh_data_p,empty_id,mutex_id,full_id);
//                    consumer();
//					return 0;
//				}
//			}
//		}
//	} else {
//        /* Child Process: Producer */
//		//producer();
//		/* Child Processes: Producers */
//		for ( i = 0; i < m; ++i ) {
//			producer_pids[i] = fork();
//			if (producer_pids[i] < 0) {
//				fprintf(stderr,"[ERROR] Failed to fork()\n");
//				return 1;
//			} else if (producer_pids[i] == 0) {
//				fprintf(stderr,"[ LOG ] I'm in Producers PID==%d, PPID==%d\n",(int) getpid(), (int) getppid());
//				//producer(sh_data_p,empty_id,mutex_id,full_id);
//                producer();
//				return 0;
//			} else {
//				/* child process which isn't producer nor consumer 
//				 * but going to be ancesstors for another producers */
//			}
//		}
//	}
//	return 0;


}
