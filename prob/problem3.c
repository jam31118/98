#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#define SEM_PATH "/sem_AOS"
//#include <features.h>

int main(int argc, char *argv[]) {
	/* Shared memory information */
	const char *shmName = "/SHM";
	size_t SIZE = sizeof(int);
	int *sh_data_p;
	int shm_fd;

	shm_fd = shm_open(shmName, O_CREAT | O_RDWR, 0666);
	if (shm_fd == -1) {printf("Shared failed in CREATing\n"); return 1;}
	ftruncate(shm_fd,SIZE);
	sh_data_p = (int *) mmap(0,SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,shm_fd,0);
	if(sh_data_p == MAP_FAILED) {printf("Map Failed\n");return 1;}

	/* Shared Data initialization */
	*sh_data_p = 0;

	/* Creating Semaphore */
	sem_t * sem_id = sem_open(SEM_PATH, O_CREAT, S_IRUSR | S_IWUSR ,1);
}
