/*
* Name: Riley Heike
* 
* Date: April 16th, 2025
* 
* Title: Lab3 - Part 3: Server Thread Program
* 
* Description: This program creates a server that accepts TCP connections, and starts a thread to allow multiple
* 		file download download requests
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUF_SIZE 2048
#define MAX_CONNECTIONS 5

struct copy_struct {
	int thread_id;
	char* src_filename;
	int sockfd;
};


int func_copy(char* src_filename, int sockfd){

	// Allocate Buffer for read	
	char* buf = malloc((size_t)BUF_SIZE);
	if (buf == NULL){
		fprintf(stderr, "Malloc Failed\n");
		close(sockfd);
		return -1;
	}

	int bytes_read, bytes_written;

	// Begin file transfer over socket connection
	int src_fd = open(src_filename, O_RDONLY);
	if (src_fd < 0){
		fprintf(stderr, "Unable to open %s for reading\n", src_filename);
		free(buf);
		close(sockfd);
		close(src_fd);
		return -1;
	}

	// Continuously read from source file and write to socket
	while((bytes_read = read(src_fd, buf, BUF_SIZE)) > 1){
		bytes_written = write(sockfd, buf, bytes_read);
		if (bytes_written < 0){
			fprintf(stderr, "Error writing to socket");
			free(buf);
			close(src_fd);
			close(sockfd);
			return -1;
		}
	}

	close(src_fd);
	close(sockfd);
	free(buf);

	return 0;

}

void* copy_thread(void *arg){
	
	// Define and fill thread parameters
	struct copy_struct* params = (struct copy_struct*)arg;
	printf("Thread [%i] - copying %s on socket %d\n", params->thread_id, params->src_filename, params->sockfd);
	func_copy(params->src_filename, params->sockfd);
	free(params->src_filename);
	free(params);
	pthread_exit(NULL);

}

int main(int argc, char* argv[]){

	// Assert program args and retrieve port number
	if (argc != 2){
		fprintf(stderr, "usage: %s port number\n", argv[0]);
		exit(1);
	}

	int port = atoi(argv[1]);
	if (port <= 1024){
		fprintf(stderr, "Invalid Port Number\n");
		exit(1);
	}

	// Define server address struct for binding
	struct sockaddr_in sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	sockaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	int sockaddr_len = sizeof(sockaddr);

	// Create socket with IPv4, TCP connection, and IP protocol
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == 0){
		perror("Socket Failed");
		exit(1);
	}

	// Bind socket to specified server address
	if (bind(sockfd, (struct sockaddr *)&sockaddr, sockaddr_len) < 0){
		perror("Bind Failed");
		close(sockfd);
		exit(1);
	}

	// Listen for client connections
	if (listen(sockfd, MAX_CONNECTIONS) < 0){
		perror("Listen Failed");
		close(sockfd);
		exit(1);
	}

	printf("Server listening on port %d...\n", port);

	int threads = 0;

	// Accept loop
	while(1){
		
		// Accept request from queue
		int new_socket = accept(sockfd, (struct sockaddr *)&sockaddr, (socklen_t *)&sockaddr_len);
		if (new_socket < 0){
			perror("Accept Failed");
			close(sockfd);
			exit(1);
		}

		printf("Successfully connection accepted from %s:%d\n", inet_ntoa(sockaddr.sin_addr), ntohs(sockaddr.sin_port));

		char src_filename[256];
		int bytes_read;

		// Await source file name to transfer
		bytes_read = read(new_socket, src_filename, sizeof(src_filename)-1);
		if (bytes_read < 0){
			perror("Error reading filename from socket\n");
			close(new_socket);
			continue;
		}
		src_filename[bytes_read] = '\0';

		printf("Client requested file %s\n", src_filename);

		// Create thread args
		struct copy_struct *thread_args = malloc(sizeof(struct copy_struct));
		if (thread_args == NULL){
			fprintf(stderr, "Malloc failed for thread args\n");
			close(new_socket);
			continue;
		}
	
		thread_args->thread_id = threads++;
		thread_args->src_filename = strdup(src_filename);
		thread_args->sockfd = new_socket;

		// Create thread and assign to copy function
		pthread_t tid;
		if(pthread_create(&tid, NULL, copy_thread, thread_args) != 0){
			perror("Failed to create thread");
			close(new_socket);
			free(thread_args);
		}
		else {
			// Allow memory to be recpaptured at termination of thread
			pthread_detach(tid);
		}

		if (threads >= MAX_CONNECTIONS){
			printf("Max connections reached, not accepting new\n");
			break;
		}

	}

	close(sockfd);

	return 0;
}

