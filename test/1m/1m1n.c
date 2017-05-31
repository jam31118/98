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
		fprintf(stderr,"[ERROR] [PID==%d] Failed to get sem value (errno == %s)\n",
				(int)getpid(),strerror(errno));	}
	if (sem_getvalue(full,&full_val)) {
		fprintf(stderr,"[ERROR] [PID==%d] Failed to get sem value (errno == %s)\n",
				(int)getpid(),strerror(errno));	}
	if (sem_getvalue(empty,&empty_val)) {
		fprintf(stderr,"[ERROR] [PID==%d] Failed to get sem value (errno == %s)\n",
				(int)getpid(),strerror(errno));	}
	fprintf(stdout,"[PID=%d] mutex=%d, full=%d, empty=%d\n",
			(int) getpid(),mutex_val, full_val, empty_val);
}

//int producer(sh_data_t *sh_data_p ,sem_t *empty, sem_t *mutex, sem_t *full){
int producer(){
	/* Open Shared memory */
	const size_t SIZE = sizeof(sh_data_t);
	int shm_fd = shm_open(SHMNAME, O_RDWR, 0666);
	if (shm_fd == -1) {fprintf(stderr,"[PID=%d]Shared failed in CREATing\n",(int)getpid()); return 1;}
	sh_data_t *sh_data_p = (sh_data_t *) 
		mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if(sh_data_p == MAP_FAILED) {fprintf(stderr,"[PID=%d] Map Failed\n",(int)getpid());return 1;}

	/* Open Semaphores */
	sem_t *empty, *mutex, *full;
	empty = sem_open(EMPTYNAME,0);
	mutex = sem_open(MUTEXNAME,0);
	full = sem_open(FULLNAME,0);
	
	/* Debugging */
//	printSEM(mutex,full,empty);

	/* Data initialization */
	int num; // idx;
	//idx =0;
	num = 1;
//	int sval_tmp;
	int in_idx = 0;
	/* Starting Iteration for Data Production */
	do{
		//fprintf(stderr,"[ LOG ] (before sema-full) I'm in Producer(PID==%d)\n",(int) getpid());
		/*
		if (sem_getvalue(empty,&sval_tmp)) {
			fprintf(stderr,"[ERROR] [PID==%d] Failed to get sem value of empty (errno == %s)\n", (int) getpid(), strerror(errno));
		} else {
			fprintf(stderr,"[ LOG ] [PID==%d] Value of empty == %d\n",(int) getpid(),sval_tmp);
			fflush(stderr);
		}*/

		if (sem_wait(empty)) {
			fprintf(stderr,"[PID=%d] Failed to sem_wait(empty)\n",(int)getpid()); 
			return 1;}
		//fprintf(stderr,"[ LOG ] (after sema-empty) I'm in Producer(PID==%d)\n",(int) getpid());
		//fflush(stderr);
		/*
		if (sem_getvalue(mutex,&sval_tmp)) {
			fprintf(stderr,"[ERROR] [PID==%d] Failed to get sem value of mutex (errno == %s)\n",(int) getpid(), strerror(errno));
		} else {
			fprintf(stderr,"[ LOG ] [PID==%d] Value of mutex == %d\n",(int) getpid(),sval_tmp);
			fflush(stderr);
		}*/

		if (sem_wait(mutex)) {
			fprintf(stderr,"[PID=%d] Failed to sem_wait(mutex)\n",(int)getpid());
			return 1; }

		fprintf(stdout,"\n=======PRODUCER-MUTEX-START======[SUM=%d][PID=%d]\n",
				sh_data_p->SUM, (int) getpid());
//		fprintf(stderr,"[ LOG ] (after mutex) I'm in Producer(PID==%d), num == %d, buffer == %p\n",(int) getpid(),num,sh_data_p->buffer);

		//*sh_data_p->write_ptr = num; 
		/*
		sh_data_p->buffer[sh_data_p->write_idx % BUFSIZE] = num;
		num +=1;
		//sh_data_p->write_ptr = (sh_data_p->buffer[((sh_data_p->write_idx)%20)]);
		sh_data_p->write_idx += 1;
		
		fprintf(stderr,"write_idx == %d, write_idx 'mod' BUFSIZE == %d\n",
				sh_data_p->write_idx, ((sh_data_p->write_idx)%20));
		printBuffer(sh_data_p->buffer);
		*/
		//sh_data_p->SUM += 1;
		in_idx = sh_data_p->write_idx % BUFSIZE;

		sh_data_p->buffer[in_idx] = num;

		fprintf(stdout,"[PID=%d] sh_data_p->write_idx == %d\n",(int)getpid(),sh_data_p->write_idx);
		fprintf(stdout,"[PID=%d] in_idx == %d\n",(int)getpid(),in_idx);
		printBuffer(sh_data_p->buffer);

		sh_data_p->write_idx++;
		num++;
		
		printSEM(mutex,full,empty);
		fprintf(stdout,"=======PRODUCER-MUTEX-ENDS=======[SUM=%d][PID=%d]\n",
				sh_data_p->SUM, (int) getpid());
	//	sem_post(mutex);
		if (sem_post(mutex)) {
			fprintf(stderr,"[PID==%d] Failed to Producer sem_post(mutex)\n",(int) getpid());
			return 1; }
		/*
		if (sem_getvalue(mutex,&sval_tmp)) {
			fprintf(stderr,"[ERROR] [PID==%d] Failed to get sem value of mutex (errno == %s)\n",
					(int) getpid(), strerror(errno));
		} else {
			fprintf(stderr,"[ LOG ] [PID==%d] [After POST] Value of mutex == %d\n",(int) getpid(),sval_tmp);
			fflush(stderr);
		}*/
		if (sem_post(full)) {
			fprintf(stderr,"[PID==%d] Failed to Producer sem_post(full)\n",(int) getpid());
			return 1; }
		
		/* Break Condition */
		if (num > MAXNUM) break;
		//fprintf(stderr,"[ LOG ] BUFFER in Producer(PID==%d) \n",(int) getpid());
		//fprintf(stderr,"\n\n\n");
		//fflush(stderr);
	} while (1);

	/* Unlink Shared memory */
//	shm_unlink(SHMNAME); 
//	One should not unlink for sake of other processes
//	Since shm_unlink() destroies the shared memory object permanently!
	
	/* Close Shared memory */
	close(shm_fd);

	/* Close Semaphores */
	sem_close(empty);
//	sem_unlink(EMPTYNAME);
	sem_close(mutex);
//	sem_unlink(MUTEXNAME);
	sem_close(full);
//	sem_unlink(FULLNAME);

	return 0;
}

//int consumer(sh_data_t *sh_data_p, sem_t *empty, sem_t *mutex , sem_t *full) {
int consumer() {
	/* Open Shared memory */
	const size_t SIZE = sizeof(sh_data_t);
	int shm_fd = shm_open(SHMNAME, O_RDWR,0666);
	if (shm_fd == -1) {fprintf(stderr,"[PID=%d]Shared failed in CREATing\n",(int)getpid()); return 1;}
	sh_data_t *sh_data_p = (sh_data_t *) 
		mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if(sh_data_p == MAP_FAILED) {printf("Map Failed\n");return 1;}

	/* Open Semaphores */
	sem_t *empty, *mutex, *full;
	empty = sem_open(EMPTYNAME,0);
	mutex = sem_open(MUTEXNAME,0);
	full = sem_open(FULLNAME,0);

	/* Debugging */
//	printSEM(mutex,full,empty);

	/* Data Initialization */
	int out_idx = 0;
	//int idx; idx = 0;
//	int sval_tmp;
	
	/* Start Interation for Data Consumption */
	do{
		//fprintf(stderr,"[ LOG ] (before sema-full) I'm in Consumer(PID==%d)\n",(int) getpid());
		/*
		if (sem_getvalue(full,&sval_tmp)) {
			fprintf(stderr,"[ERROR] [PID==%d] Failed to get sem value of full (errno == %s)\n",
					(int) getpid(), strerror(errno));
		} else {
			fprintf(stderr,"[ LOG ] [PID==%d] Value of Consumer full == %d\n",(int) getpid(),sval_tmp);
		}*/
		if (sem_wait(full)) {fprintf(stderr,"[PID=%d] Failed to sem_wait(full)\n",(int)getpid());}
		//fprintf(stderr,"[ LOG ] (after sema-full) I'm in Consumer(PID==%d)\n",(int) getpid());
		/*
		if (sem_getvalue(mutex,&sval_tmp)) {
			fprintf(stderr,"[ERROR] [PID==%d] Failed to get sem value of mutex (errno == %s)\n",
					(int) getpid(), strerror(errno));
		} else {
			fprintf(stderr,"[ LOG ] [PID==%d] Value of Consumer mutex == %d\n",(int) getpid(),sval_tmp);
			fflush(stderr);
		}*/
		if (sem_wait(mutex)) {fprintf(stderr,"[PID=%d] Failed to sem_wait(mutex)\n",(int)getpid());}
		
		fprintf(stdout,"\n+++++CONSUMER-MUTEX-START+++++++[SUM=%d][PID=%d]\n",
				sh_data_p->SUM, (int) getpid());
		/*
		fprintf(stderr,"[ LOG ] (after mutex) I'm in Consumer(PID==%d)\n",(int) getpid());
		sh_data_p->SUM	+=	*sh_data_p->read_ptr;
		*sh_data_p->read_ptr = 0;
		sh_data_p->read_ptr = (sh_data_p->buffer+((sh_data_p->read_idx++)%20));
			
		printBuffer(sh_data_p->buffer);
		*/
		printSEM(mutex,full,empty);
        
        /* Determine index to read from buffer */
		out_idx = sh_data_p->read_idx % BUFSIZE;
        
        /* Read and use the data */
        sh_data_p->SUM += sh_data_p->buffer[out_idx];

        /* Clearing the buffer section that has already been read */
		sh_data_p->buffer[out_idx] = 0;

		fprintf(stdout,"[PID=%d] read_idx = %d\n",(int)getpid(),sh_data_p->read_idx);
		fprintf(stdout,"[PID=%d] out_idx == %d\n",(int)getpid(),out_idx);
		printBuffer(sh_data_p->buffer);
        
        /* Increment the reading index to next element of buffer */
		sh_data_p->read_idx += 1; // 170529 Problem! (170529 solved)

		fprintf(stdout,"+++++CONSUMER-MUTEX-ENDS++++++++[SUM=%d][PID=%d]\n\n",
				sh_data_p->SUM, (int) getpid());
	//	sem_post(mutex);
		if (sem_post(mutex)) {fprintf(stderr,"[PID==%d] Failed to Consumer sem_post(mutex)\n",(int) getpid());}
		/*
		if (sem_getvalue(mutex,&sval_tmp)) {
			fprintf(stderr,"[ERROR] [PID==%d] Failed to get sem value of mutex (errno == %s)\n",
					(int) getpid(), strerror(errno));
		} else {
			fprintf(stderr,"[ LOG ] [PID==%d] [After POST] Value of mutex == %d\n",(int) getpid(),sval_tmp);
			fflush(stderr);
		}*/
		if (sem_post(empty)) {fprintf(stderr,"[PID==%d] Failed to Consumer sem_post(empty)\n",(int) getpid());}
		
		/* Break Condition */
		if ((sh_data_p->write_idx >= MAXNUM) && isEmpty(sh_data_p->buffer,BUFSIZE)) break; // 170529-1517 Problem .. should read all (170601 solved!) 
//		fprintf(stderr,"\n\n\n");
//		fprintf(stderr,"[ LOG ] BUFFER in Producer(PID==%d) ",(int) getpid());
	} while (1);
	//} while (sh_data_p->write_idx < 50);
	
	/* Unlink Shared memory */
//	shm_unlink(SHMNAME);

	/* Close Shared memory */
	close(shm_fd);

	/* Close Semaphores */
	sem_close(empty);
//	sem_unlink(EMPTYNAME);
	sem_close(mutex);
//	sem_unlink(MUTEXNAME);
	sem_close(full);
//	sem_unlink(FULLNAME);

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
	setbuf(stderr,NULL);
	setbuf(stdout,NULL);

	/* Shared memory information */
	const char *shmName = "/SHM";
	size_t SIZE = sizeof(sh_data_t);
	sh_data_t *sh_data_p;
	int shm_fd;
	
	/* Shared memory declaration */
	shm_fd = shm_open(shmName, O_CREAT | O_RDWR, 0666);
	if (shm_fd == -1) {fprintf(stderr,"[PID=%d]Shared failed in CREATing\n",(int)getpid()); return 1;}
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
		
	/* Semaphore information */
	/*
	const char *fullName = "/FULL";
	const char *mutexName = "/MUTEX";
	const char *emptyName = "/EMPTY";
	*/
	
	/* Semaphore ID generation */
	sem_t *full_id = sem_open(FULLNAME, O_CREAT, S_IRUSR | S_IWUSR, 0);
	sem_t *empty_id = sem_open(EMPTYNAME, O_CREAT, S_IRUSR | S_IWUSR, BUFSIZE);
	if (empty_id == SEM_FAILED) {fprintf(stderr,"empty_id is failed\n");}
	sem_t *mutex_id = sem_open(MUTEXNAME, O_CREAT, S_IRUSR | S_IWUSR, 1);

	sem_init(full_id,10,0);
	sem_init(empty_id,10,BUFSIZE);
	sem_init(mutex_id,10,1);

	/* Debug */
	printSEM(mutex_id,full_id,empty_id); // check initialization

//	int sval_tmp = -1;
	/*
	if (sem_getvalue(empty_id,&sval_tmp)) {
		fprintf(stderr,"[ERROR] [PID==%d] Failed to get sem value of empty (errno == %s)\n",
				(int) getpid(), strerror(errno));
	} else {
		fprintf(stderr,"[ LOG ] [PID==%d] Value of empty == %d\n",(int) getpid(),sval_tmp);
	}	
	if (sem_getvalue(full_id,&sval_tmp)) {
		fprintf(stderr,"[ERROR] [PID==%d] Failed to get sem value of full (errno == %s)\n",
				(int) getpid(), strerror(errno));
	} else {
		fprintf(stderr,"[ LOG ] [PID==%d] Value of full == %d\n",(int) getpid(),sval_tmp);
	}*/	


	/* Creating Child Processes */
	pid_t ch1, ch2;

	ch1 = fork();
	int status = -1;
	pid_t tmp;
	if (ch1) {
		/* Parent Process */
		ch2 = fork();
		if (ch2) {
			/* Parent Process in deeper but same */
			/*
			while (procAliveNum>0) {
				wait_result_pids[totalProcNum - procAliveNum] = wait(&status);
				--procAliveNum;
				fprintf(stderr,"[ LOG ] Child(PID==%d) exited with status 0x%x\n",
						wait_result_pids[totalProcNum - procAliveNum], status);
			}*/
			/*
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
			}*/
			//tmp = waitpid(ch1, &status, 0);
			tmp = wait(&status);
			fprintf(stderr,"[PID=%d] Producers(PID==%d) exited with status %d\n",
					(int) getpid(), tmp, status);
			//tmp = waitpid(ch1, &status, 0);
			tmp = wait(&status);
			fprintf(stderr,"[PID=%d] Consumers(PID==%d) exited with status %d\n",
					(int) getpid(), tmp, status);	

			/* Unlink Shared memory */
			shm_unlink(SHMNAME);
		
			/* Close and Unlink Semaphores */
			sem_close(empty_id);
			sem_unlink(EMPTYNAME);
			sem_close(mutex_id);
			sem_unlink(MUTEXNAME);
			sem_close(full_id);
			sem_unlink(FULLNAME);

			printf("%d\n",sh_data_p->SUM);
		} else {
			/* Child Processes: Consumers */
			/*
			for ( i = 0; i < n; ++i ) {
				consumer_pids[i] = fork();
				if (consumer_pids[i] < 0) {
					fprintf(stderr,"[ERROR] Failed to fork()\n");
					return 1;
				} else if (consumer_pids[i] == 0) {
					fprintf(stderr,"[ LOG ] I'm in Comsumers PID==%d, PPID==%d, SUM == %d\n",
							(int) getpid(), (int) getppid(), sh_data_p->SUM);
					fflush(stderr);
					consumer(sh_data_p,empty_id,mutex_id,full_id);
					return 0;
				}
			}*/
			//consumer(sh_data_p,empty_id,mutex_id,full_id);
			//consumer(sh_data_p);
			consumer();
		}

	} else {
		/* Child Processes: Producers */
		/*
		for ( i = 0; i < m; ++i ) {
			producer_pids[i] = fork();
			if (producer_pids[i] < 0) {
				fprintf(stderr,"[ERROR] Failed to fork()\n");
				return 1;
			} else if (producer_pids[i] == 0) {
				fprintf(stderr,"[ LOG ] I'm in Producers PID==%d, PPID==%d, SUM == %d\n",
						(int) getpid(), (int) getppid(), sh_data_p->SUM);
				fflush(stderr);
				return 0;
			}
		}*/
		//producer(sh_data_p,empty_id,mutex_id,full_id);
		//producer(sh_data_p);
		producer();
	}
	return 0;
}
