/*
* Name: Riley Heike
* 
* Date: April 16th, 2025
* 
* Title: Lab3 - Part 1: Server Program
* 
* Description: This program creates a server that accepts TCP connections and provides
* 		a file for download
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUF_SIZE 2048
#define MAX_CONNECTIONS 5

int main(int argc, char* argv[]){

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

	// Extract first connection request 
	int new_socket = accept(sockfd, (struct sockaddr *)&sockaddr, (socklen_t *)&sockaddr_len);
	if (new_socket < 0){
		perror("Accept Failed");
		close(sockfd);
		exit(1);
	}

	printf("Successfully connection accepted from %s:%d\n", inet_ntoa(sockaddr.sin_addr), ntohs(sockaddr.sin_port));

	// Allocate buffer for read/write
	char* buf = malloc((size_t)BUF_SIZE);
	if (buf == NULL){
		fprintf(stderr, "Malloc Failed\n");
		close(new_socket);
	}

	char src_filename[256];
	int bytes_read, bytes_written;

	// Await source file name to transfer
	bytes_read = read(new_socket, src_filename, sizeof(src_filename)-1);
	if (bytes_read < 0){
		perror("Error reading from socket\n");
	}
	src_filename[bytes_read] = '\0';

	printf("Client requested file %s\n", src_filename);

	// Begin file transfer over socket connection
	int src_fd = open(src_filename, O_RDONLY);
	if (src_fd < 0){
		fprintf(stderr, "Unable to open %s for reading\n", src_filename);
		free(buf);
		close(new_socket);
		close(src_fd);
	}

	// Continuously read from source file and copy to socket
	while((bytes_read = read(src_fd, buf, BUF_SIZE)) > 0){
		bytes_written = write(new_socket, buf, bytes_read);
		if (bytes_written < 0){
			fprintf(stderr, "Error writing to socket");
			free(buf);
			close(src_fd);
			close(new_socket);
			exit(1);
		}
	}

	close(src_fd);
	close(new_socket);
	free(buf);

	return 0;
}

