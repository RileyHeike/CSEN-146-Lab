/*
* Name: Riley Heike
* 
* Date: April 16th, 2025
* 
* Title: Lab3 - Part 2: Client Program
* 
* Description: This program creates a client that connects to a server through TCP connection and requests
* 		a file for download
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

#define BUF_SIZE 2048

int socket_read(int sockfd, const char* dst_filename){

	// Open destination file descriptor for copying
	int dst_fd = open(dst_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (dst_fd < 0){
		perror("Failed to open the destination file\n");
		close(sockfd);
		return 1;
	}

	char* buf = malloc((size_t)BUF_SIZE);
	int bytes_read, bytes_written;

	// Continuously read from the socket and copy to dst file
	while((bytes_read = read(sockfd, buf, BUF_SIZE)) > 0){
		bytes_written = write(dst_fd, buf, bytes_read);
		if (bytes_written < 0){
			perror("Write to dst file failed\n");
			close(sockfd);
			close(dst_fd);
			free(buf);
			return 1;
		}
	}

	printf("File recieved and saved in %s\n", dst_filename);

	close(sockfd);
	close(dst_fd);
	return 0;
}

int main(int argc, char* argv[]){

	if (argc != 5){
		fprintf(stderr, "usage: %s IPAddress PortNo src dst\n", argv[0]);
		exit(1);
	}

	// Translate args to variables
	const char* server_ip = argv[1];
	int port = atoi(argv[2]);
	const char* src_filename = argv[3];
	const char* dst_filename = argv[4];

	// Create socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		perror("Socket Creation Failed");
		exit(1);
	}

	// Fill in server address params
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);

	if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0){
		perror("Invalid address");
		exit(1);
	}

	// Attempt socket connection
	if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		perror("Connection failed");
		exit(1);
	}

	// Write source file name to server
	write(sockfd, src_filename, strlen(src_filename));

	// Copy to dst file
	socket_read(sockfd, dst_filename);

	return 0;

}
