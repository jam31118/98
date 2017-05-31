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
	fprintf(stderr,"[[");
	fflush(stderr);
	for(buf_p = buffer; buf_p<buffer+BUFSIZE; buf_p++) {
		fprintf(stderr,"%3d",*buf_p);
		fflush(stderr);
	}
	fprintf(stderr,"]]");
	fflush(stderr);
	fprintf(stderr,"\n");
	fflush(stderr);
}

//int producer(sh_data_t *sh_data_p ,sem_t *empty, sem_t *mutex, sem_t *full){
int producer(sh_data_t *sh_data_p){
	int num; // idx;
	//idx =0;
	num = 1;
	int sval_tmp;
	sem_t *empty, *mutex, *full;
	empty = sem_open(EMPTYNAME,0);
	mutex = sem_open(MUTEXNAME,0);
	full = sem_open(FULLNAME,0);
	do{
		//fprintf(stderr,"[ LOG ] (before sema-full) I'm in Producer(PID==%d)\n",(int) getpid());
		if (sem_getvalue(empty,&sval_tmp)) {
			fprintf(stderr,"[ERROR] [PID==%d] Failed to get sem value of empty (errno == %s)\n", (int) getpid(), strerror(errno));
		} else {
			fprintf(stderr,"[ LOG ] [PID==%d] Value of empty == %d\n",(int) getpid(),sval_tmp);
			fflush(stderr);
		}

		sem_wait(empty);
		//fprintf(stderr,"[ LOG ] (after sema-empty) I'm in Producer(PID==%d)\n",(int) getpid());
		fflush(stderr);

		if (sem_getvalue(mutex,&sval_tmp)) {
			fprintf(stderr,"[ERROR] [PID==%d] Failed to get sem value of mutex (errno == %s)\n",(int) getpid(), strerror(errno));
		} else {
			fprintf(stderr,"[ LOG ] [PID==%d] Value of mutex == %d\n",(int) getpid(),sval_tmp);
			fflush(stderr);
		}

		sem_wait(mutex);
		fprintf(stderr,"\n\n\n=======PRODUCER-MUTEX-START========\n");
		fprintf(stderr,"[ LOG ] (after mutex) I'm in Producer(PID==%d), num == %d, buffer == %p\n",(int) getpid(),num,sh_data_p->buffer);
		
			fflush(stderr);

		//*sh_data_p->write_ptr = num; 
		sh_data_p->buffer[sh_data_p->write_idx % BUFSIZE] = num;
		num +=1;
		//sh_data_p->write_ptr = (sh_data_p->buffer[((sh_data_p->write_idx)%20)]);
		sh_data_p->write_idx += 1;
		
		fprintf(stderr,"write_idx == %d, write_idx 'mod' BUFSIZE == %d\n",
				sh_data_p->write_idx, ((sh_data_p->write_idx)%20));
		printBuffer(sh_data_p->buffer);
		
		fprintf(stderr,"=======PRODUCER-MUTEX-ENDS==========\n");
	//	sem_post(mutex);
		if (sem_post(mutex)) {fprintf(stderr,"[PID==%d] Failed to Producer sem_post(mutex)\n",(int) getpid());}
		if (sem_getvalue(mutex,&sval_tmp)) {
			fprintf(stderr,"[ERROR] [PID==%d] Failed to get sem value of mutex (errno == %s)\n",
					(int) getpid(), strerror(errno));
		} else {
			fprintf(stderr,"[ LOG ] [PID==%d] [After POST] Value of mutex == %d\n",(int) getpid(),sval_tmp);
			fflush(stderr);
		}
		if (sem_post(full)) {fprintf(stderr,"[PID==%d] Failed to Producer sem_post(full)\n",(int) getpid());}
		
		fprintf(stderr,"[ LOG ] BUFFER in Producer(PID==%d) \n",(int) getpid());
		fprintf(stderr,"\n\n\n");
		fflush(stderr);
	} while (num<=50);
	return 0;
}

//int consumer(sh_data_t *sh_data_p, sem_t *empty, sem_t *mutex , sem_t *full) {
int consumer(sh_data_t *sh_data_p) {
	//int idx; idx = 0;
	int sval_tmp;
	sem_t *empty, *mutex, *full;
	empty = sem_open(EMPTYNAME,0);
	mutex = sem_open(MUTEXNAME,0);
	full = sem_open(FULLNAME,0);
	do{
		//fprintf(stderr,"[ LOG ] (before sema-full) I'm in Consumer(PID==%d)\n",(int) getpid());
		if (sem_getvalue(full,&sval_tmp)) {
			fprintf(stderr,"[ERROR] [PID==%d] Failed to get sem value of full (errno == %s)\n",
					(int) getpid(), strerror(errno));
		} else {
			fprintf(stderr,"[ LOG ] [PID==%d] Value of Consumer full == %d\n",(int) getpid(),sval_tmp);
		}
		sem_wait(full);
		//fprintf(stderr,"[ LOG ] (after sema-full) I'm in Consumer(PID==%d)\n",(int) getpid());
		//
		if (sem_getvalue(mutex,&sval_tmp)) {
			fprintf(stderr,"[ERROR] [PID==%d] Failed to get sem value of mutex (errno == %s)\n",
					(int) getpid(), strerror(errno));
		} else {
			fprintf(stderr,"[ LOG ] [PID==%d] Value of Consumer mutex == %d\n",(int) getpid(),sval_tmp);
			fflush(stderr);
		}
		sem_wait(mutex);
		
		fprintf(stderr,"\n\n\n+++++CONSUMER-MUTEX-START++++++++\n");
		fprintf(stderr,"[ LOG ] (after mutex) I'm in Consumer(PID==%d)\n",(int) getpid());
		sh_data_p->SUM	+=	*sh_data_p->read_ptr;
		*sh_data_p->read_ptr = 0;
		sh_data_p->read_ptr = (sh_data_p->buffer+((sh_data_p->read_idx++)%20));
			
		printBuffer(sh_data_p->buffer);

		fprintf(stderr,"+++++CONSUMER-MUTEX-ENDS++++++++\n\n\n");
	//	sem_post(mutex);
		if (sem_post(mutex)) {fprintf(stderr,"[PID==%d] Failed to Consumer sem_post(mutex)\n",(int) getpid());}
		if (sem_getvalue(mutex,&sval_tmp)) {
			fprintf(stderr,"[ERROR] [PID==%d] Failed to get sem value of mutex (errno == %s)\n",
					(int) getpid(), strerror(errno));
		} else {
			fprintf(stderr,"[ LOG ] [PID==%d] [After POST] Value of mutex == %d\n",(int) getpid(),sval_tmp);
			fflush(stderr);
		}
		if (sem_post(empty)) {fprintf(stderr,"[PID==%d] Failed to Consumer sem_post(empty)\n",(int) getpid());}

		fprintf(stderr,"\n\n\n");
//		fprintf(stderr,"[ LOG ] BUFFER in Producer(PID==%d) ",(int) getpid());
	} while (sh_data_p->write_idx < 50);
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
	/* Parsing */
//	if (argc <= 1) {printf("Enter m, n values\n");return 1;}
//	int m = atoi(argv[1]);
//	int n = atoi(argv[2]);

//	int totalProcNum = m + n;
	//int procAliveNum = totalProcNum;
//	pid_t producer_pids[m], consumer_pids[n];
//	pid_t wait_result_pids[totalProcNum];

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
	sh_data_p->write_idx = 0;
	sh_data_p->read_idx = 0;
		
	/* Semaphore information */
	const char *fullName = "/FULL";
	const char *mutexName = "/MUTEX";
	const char *emptyName = "/EMPTY";
	
	/* Semaphore ID generation */
	sem_t *full_id = sem_open(fullName, O_CREAT, S_IRUSR | S_IWUSR, 0);
	sem_t *empty_id = sem_open(emptyName, O_CREAT, S_IRUSR | S_IWUSR, BUFSIZE);
	if (empty_id == SEM_FAILED) {fprintf(stderr,"empty_id is failed\n");}
	sem_t *mutex_id = sem_open(mutexName, O_CREAT, S_IRUSR | S_IWUSR, 1);

	sem_init(full_id,10,0);
	sem_init(empty_id,10,BUFSIZE);
	sem_init(mutex_id,10,1);

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
			fprintf(stderr,"[ LOG ] Producers(PID==%d) exited with status 0x%x\n",
					tmp, status);
			//tmp = waitpid(ch1, &status, 0);
			tmp = wait(&status);
			fprintf(stderr,"[ LOG ] Consumers(PID==%d) exited with status 0x%x\n",
					tmp, status);	
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
			consumer(sh_data_p);
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
		producer(sh_data_p);
	}
	return 0;
}
