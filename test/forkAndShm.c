#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>

int main() {
	/* Shared memory information */
	const char *shmName = "/SHM";
	size_t SIZE = sizeof(int);
	int *sh_data_p;
	int shm_fd;

	pid_t pid;
	pid = fork();

	if (pid == 0) {
		/* Child Process */
		printf("I'm the CHILD! (PID == %d)\n",(int) getpid());
		shm_fd = shm_open(shmName,O_RDONLY,0666);
		if (shm_fd == -1) {printf("Shared failed in CHILD\n"); return 1;}
		sh_data_p = (int *) mmap(0,SIZE,PROT_READ, MAP_SHARED,shm_fd,0);
		if(sh_data_p == MAP_FAILED) {
			printf("Map Failed in CHILD\n");
			return 1;
		}
		*sh_data_p = 1001;
		munmap(sh_data_p,SIZE);
	} else {
		/* Parent Process */
		printf("I'm the PARENT! (PID == %d)\n",(int) getpid());
		shm_fd = shm_open(shmName,O_CREAT | O_RDWR, 0666);
		if (shm_fd == -1) {printf("Shared failed in PARENT\n"); return 1;}
		ftruncate(shm_fd,SIZE);
		sh_data_p = (int *) mmap(0,SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,shm_fd,0);
		if(sh_data_p == MAP_FAILED) {
			printf("Map Failed in PARENT\n");
			return 1;
		}
		printf("data == %d\n",*sh_data_p);
		if (shm_unlink(shmName) == -1) {printf("Error removing %s\n",shmName); return 1;}
	}
	
	return 0;
}
