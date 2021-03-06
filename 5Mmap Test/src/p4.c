/*
 * p4.c
 *
 *  Created on: Dec 3, 2017
 *      Author: scott
 */
#include <inttypes.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#define FILENAME "something_in_the_middle"

int createFile(size_t length) {
	char c = 'A';
	int fd = open(FILENAME, O_RDWR|O_CREAT|O_TRUNC, 0666);
	if(fd==-1){
        fprintf(stderr,"Failed to open testfile[%s]: %s\n",FILENAME,strerror(errno));
		exit(EXIT_FAILURE);
	}
    	for(int i=0; i < length; i++) {
    		if(write(fd, &c, 1)!=1){
    			fprintf(stderr,"Failed to write 'A' of filesize for testfile[%s]: %s\n",FILENAME,strerror(errno));
    			exit(EXIT_FAILURE);
    		}
    	}
    	if(lseek(fd, 0, SEEK_SET)==-1){
		fprintf(stderr,"Failed to lseek back to the start of filesize for testfile[%s]: %s\n",FILENAME,strerror(errno));
		exit(EXIT_FAILURE);
    	}
    	return fd;
}
size_t GetFileSize(const char* filename) {
	struct stat fileStat;
	if (stat(filename, &fileStat) != 0) {
        fprintf(stderr,"Unable to get size of testfile[%s]: %s\n",FILENAME,strerror(errno));
        exit(EXIT_FAILURE);
	} else {
		fprintf(stderr,"TestFile[%s] size is: %jd\n",FILENAME, (intmax_t)fileStat.st_size);
		return  fileStat.st_size;
	}
}
int main(int argc, char ** argv ){
	int page_size = getpagesize (  );
	fprintf(stderr,"Page size is: %d\n",page_size);
	int flag;
	off_t fileSize;
    off_t newfileSize;
    size_t length=8195;
	int fd=createFile(length);
	fileSize=GetFileSize(FILENAME);
    fprintf(stderr,"Creating mapped_area[PROT_WRITE | PROT_READ] with size : %jd\n",(intmax_t)(fileSize));
	char * mapped_area = mmap(NULL,(size_t)fileSize, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	if(mapped_area==MAP_FAILED){
		 fprintf(stderr,"Failed to mmap testfil[%s]: %s\n",FILENAME,strerror(errno));
		 exit(EXIT_FAILURE);
	}
	mapped_area[fileSize] = 'e';
	if(munmap(mapped_area,(size_t) fileSize)==-1){
		fprintf(stderr,"Failed to munmap testfile[%s]: %s\n",FILENAME,strerror(errno));
		exit(EXIT_FAILURE);
	}
	newfileSize=GetFileSize(FILENAME);
	if (newfileSize==fileSize) {
		flag=1;
		fprintf(stderr,"When a write is made one byte beyond the size of an mmapped file, for a file with a size that is not a multiple of of the page size, its size as reported by stat(2) does not change.\n");
	} else {
		flag=0;
		fprintf(stderr,"When a write is made one byte beyond the size of an mmapped file, for a file with a size that is not a multiple of of the page size, its size as reported by stat(2) changes.\n");
	}
	if(close(fd)==-1){
		fprintf(stderr,"Failed to close testfile[%s]: %s\n",FILENAME,strerror(errno));
		exit(EXIT_FAILURE);
	}
	exit(flag);
	return 0;
}
