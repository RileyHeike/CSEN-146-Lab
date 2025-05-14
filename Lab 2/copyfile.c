/*
* Name: Riley Heike
* 
* Date: April 9th, 2025
*
* Title: Lab2 – Thread Copying
* 
* Description: This program copies a set of files from arguments using threads
*
*/

#include <stdio.h>   // fprintf(), fread(), fwrite(), fopen(), fclose()
#include <stdlib.h>  // malloc(), free()
#include <pthread.h> // pthread_create()
#include <unistd.h>  // read(), write()
#include <fcntl.h>   // open(), close()
#include <errno.h>   // errno
#include <time.h>    // clock()

#define BUF_SIZE 2048 //buffer size

// data structure to hold copy
struct copy_struct {
	int thread_id; 
	char* src_filename;
	char* dst_filename; 
};

// copies a files from src_filename to dst_filename using functions fopen(), fread(), fwrite(), fclose()
int func_copy(char* src_filename, char* dst_filename) {

	FILE* src;
	FILE* dst;

	src = fopen(src_filename, "r");	// opens a file for reading
	if (src == NULL) { // fopen() error checking
		fprintf(stderr, "unable to open %s for reading: %i\n", src_filename, errno);
		exit(0);
	}
	dst = fopen(dst_filename, "w");	// opens a file for writing; erases old file/creates a new file
	if (dst == NULL) { // fopen() error checking
		fprintf(stderr, "unable to open/create %s for writing: %i\n", dst_filename, errno);
		exit(0);
	}

	char* buf = malloc((size_t)BUF_SIZE);  // allocate a buffer to store read data
	if(buf == NULL){
		fprintf(stderr, "Malloc fail\n");
		fclose(src);
		fclose(dst);
	}

	int bytes_read, bytes_written;

	// reads content of file is loop iterations until end of file
	while((bytes_read = fread(buf, 1, BUF_SIZE, src)) > 0){
		
		// writes bytes_read to dst_filename 
		bytes_written = fwrite(buf, 1, bytes_read, dst);
		if(bytes_written != bytes_read){
			fprintf(stderr, "Error writing");
			free(buf);
			fclose(src);
			fclose(dst);
			exit(1);
		}
	}

	// closes src file pointer
	fclose(src);
	// closes dst file pointer
	fclose(dst);
	// frees memory used for buf
	free(buf);
	
	return 0;
}

// thread function to copy one file
void* copy_thread(void* arg) {
	struct copy_struct params = *(struct copy_struct*)arg;  // cast/dereference void* to copy_struct
	printf("thread[%i] - copying %s to %s\n", params.thread_id, params.src_filename, params.dst_filename);
	//call file copy function
	func_copy(params.src_filename, params.dst_filename);
	pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
	 // check correct usage of arguments in command line
	if (argc % 2 != 1) {  
		fprintf(stderr, "usage: %s <n connections> ........\n", argv[0]);
		exit(1);
	}

	int num_threads = argc / 2; // number of threads to create
	char* src_filenames[num_threads]; // array of source files
	char* dst_filenames[num_threads]; // array of desintation files
	
	//populate source and destination file arrays from argv
	int i;
	int index = 0;
	for (i = 1; i < argc; ++i){
		if(i % 2 == 1){
			src_filenames[index] = argv[i];
		}
		else {
			dst_filenames[index] = argv[i];
			index++;
		}
	}

	pthread_t threads[num_threads]; //initialize threads
	struct copy_struct thread_params[num_threads]; // structure for each thread

	//start clock
	clock_t func_start, func_end;
	func_start = clock();

	for (i = 0; i < num_threads; i++) {
		// initialize thread parameters
		thread_params[i].thread_id = i;
		thread_params[i].src_filename = src_filenames[i];
		thread_params[i].dst_filename = dst_filenames[i];
		// create each copy thread
		if (pthread_create(&threads[i], NULL, copy_thread, (void *) &thread_params[i]) != 0){
			perror("pthread_create failed");
			exit(1);
		}
	}

	// wait for all threads to finish
	for (i = 0; i < num_threads; i++) {
		pthread_join(threads[i], NULL);
	}
	

	func_end = clock();
	int func_time = (double)((func_end - func_start) / CLOCKS_PER_SEC);
	printf("Time to copy %d files: %d", num_threads, func_time);

	pthread_exit(NULL);
}
