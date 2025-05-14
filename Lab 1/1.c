/*

 Name: Riley Heike
 
 Date: April 2nd, 2025

 Title: Lab1 - Function Copy
 
 Description: This program copies the contents of a source file src to a destination file 
              using functions.

*/


#include <stdio.h>   // fprintf(), fread(), fwrite(), fopen(), fclose()
#include <stdlib.h>  // malloc(), free()
#include <pthread.h> // pthread_create()
#include <errno.h>   // errno
#include <sys/mman.h> // mmap()

#define BUF_SIZE 2048 //buffer size

// copies a files from src_filename to dst_filename using functions fopen(), fread(), fwrite(), fclose()
int func_copy(char* src_filename, char* dst_filename) {
       
	// declare source and dest file vars
	FILE* src;
	FILE* dst;

        src = fopen(src_filename, "r"); // opens a file for reading
        if (src == NULL) { // fopen() error checking
                fprintf(stderr, "unable to open %s for reading: %i\n", src_filename, errno);
                exit(0);
        }
        dst = fopen(dst_filename, "w"); // opens a file for writing; erases old file/creates a new file
        if (dst == NULL) { // fopen() error checking
                fprintf(stderr, "unable to open/create %s for writing: %i\n", dst_filename, errno);
                fclose(src);
		exit(0);
        }

        char* buf = malloc((size_t)BUF_SIZE);  // allocate a buffer to store read data
        //ensure buf allocatedd correctly
	if (buf == NULL) {
		fprintf(stderr, "malloc fail\n");
		fclose(src);
		fclose(dst);
		exit(0);
	}
	
	// define var for chars read and written
	size_t bytes_read, bytes_written;

	// reads content of file is loop iterations until end of file
       	while((bytes_read = fread(buf, 1, BUF_SIZE, src)) > 0){
        	
		// writes bytes_read to dst_filename 
		bytes_written = fwrite(buf, 1, bytes_read, dst);
	
		// error check
		if(bytes_written != bytes_read){
			fprintf(stderr, "Error writing");
			free(buf);
			fclose(src);
			fclose(dst);
			exit(0);
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

int main(int argc, char* argv[]) {
        if (argc != 3) {  // check correct usage
                fprintf(stderr, "usage: %s <src_filename> <dst_filename>\n", argv[0]);
                exit(1);
        }
        // get the source and destination files from the command line arguments
	char* src = argv[1];
	char* dst = argv[2];

	// call the function copy function
	func_copy(src, dst);

	return 0;
}

