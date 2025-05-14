/*

 Name: Riley Heike
 
 Date: April 2nd, 2025

 Title: Lab1 - Function + System Copy Times
 
 Description: This program copies the contents of a source file src to a destination file 
 	      using two different methods and compares the time required to complete each.

*/


#include <stdio.h>   // fprintf(), fread(), fwrite(), fopen(), fclose()
#include <stdlib.h>  // malloc(), free()
#include <pthread.h> // pthread_create()
#include <unistd.h>  // read(), write()
#include <fcntl.h>   // open(), close()
#include <errno.h>   // errno
#include <time.h>    // clock()
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

// copies a files form src_filename to dst_filename using syscalls open(), read(), write(), close()
int syscall_copy(char* src_filename, char* dst_filename)  {
        int src_fd = open(src_filename, O_RDONLY);  // opens a file for reading
        if (src_fd < 0) { // open() error checking
                fprintf(stderr, "unable to open %s for reading: %i\n", src_filename, errno);
                close(src_fd);
                exit(0);
        }

        // O_WRONLY - write only
        // O_CREAT - create file if it doesn't exist
        // O_TRUNC - truncates file to length 0 if it already exists (deletes existing file)
        // opens a file for writing; erases old file/creates a new file
        int dst_fd = open(dst_filename, O_WRONLY | O_CREAT | O_TRUNC);
        
	// open() error checking
	if (dst_fd < 0) {
                fprintf(stderr, "unable to open/create %s for writing: %i\n", dst_filename, errno);
                close(dst_fd);
                exit(0);
        }

        // allocate a buffer to store read data
        char* buf = malloc((size_t)BUF_SIZE);  
        int nr, nw;

	// read/ write loop
        // reads up to BUF_SIZE bytes from src_filename
        while((nr = read(src_fd, buf, BUF_SIZE)) > 0){
                
		// writes bytes_read to dst_filename
                nw = write(dst_fd, buf, nr);
                
		// error check
                if (nw < 0) {
                        fprintf(stderr, "Error writing");
                        free(buf);
                        close(src_fd);
                        close(dst_fd);
                        exit(1);
                }
        }


        // closes src_fd file descriptor
        close(src_fd);
        // closes dst_fd file descriptor
        close(dst_fd);
        // frees memory used for buf
        free(buf);

        return 0;
}


// checks the runtime to copy a file using functions vs syscalls
void check_copy_times(char* src_filename, char* dst_filename) {
        
	// declare vars
	clock_t func_start, func_end, syscall_start, syscall_end;
	double func_time, syscall_time;

        // capture runtime of func_copy() using start clock, call the copy, end the clock
	func_start = clock();
	func_copy(src_filename, dst_filename);	
	func_end = clock();
	func_time = (double)(func_end - func_start) / CLOCKS_PER_SEC;

        // capture runtime of syscall_copy() using start clock, call the copy, end the clock
	syscall_start = clock();
	syscall_copy(src_filename, dst_filename);
	syscall_end = clock();
	syscall_time = (double)(syscall_end - syscall_start) / CLOCKS_PER_SEC;

	// print times
        printf("time to copy using functions: %.7f\n", func_time);
        printf("time to copy using syscalls: %.7f\n", syscall_time);
}

int main(int argc, char* argv[]) {
        
	// check correct usage
	if (argc != 3) {  
                fprintf(stderr, "usage: %s <src_filename> <dst_filename>\n", argv[0]);
                exit(1);
        }

        //get the source and destination files from the command line arguments
        char* src = argv[1];
        char* dst = argv[2];

	//call the check copy times function
	check_copy_times(src, dst);

        return 0;

}
