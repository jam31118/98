#include <stdio.h>
#include <sys/shm.h>
#include <sys/mman.h>
//#include <sys/stat.h>
#include <fcntl.h>
//#include <stdlib.h>
//#include <string.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
	const char *shm_name = "/TEST";
	int N = 1;
	size_t SIZE = sizeof(int)*N;

	int data = 3;
	
	int i; 
	int shm_fd;
	int *ptr;
	shm_fd = shm_open(shm_name,O_CREAT | O_RDWR, 0666);
//	printf("shm_fd == %d\n",shm_fd);

//	ftruncate(shm_fd,sizeof(data));
	ptr = (int *) mmap(0,SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,shm_fd,0);
//	printf("ptf == %p\n",ptr);

	*ptr = data;

	for (i=0; i<N; i++) {
		printf("DATA[%d] == %d\n",i,*(ptr+i));
	} printf("\n");

	munmap(ptr,SIZE);
}
