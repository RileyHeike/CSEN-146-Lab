/*
* Name: Riley Heike
* 
* Date: April 25th, 2025
* 
* Title: Lab 4 - Part 1: Server Program
* 
* Description: This program recieves a file from a client through UDP socket
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

#define FILE_NAME "dst.dat"
#define BUF_SIZE 10

struct Packet {
	int len;
	char buf[BUF_SIZE];
};

int main(int argc, char* argv[]){

	if (argc != 2){
		fprintf(stderr, "usage: %s port", argv[0]);
		exit(1);
	}
	int port = atoi(argv[1]);

	// Define server address struct for binding
	struct sockaddr_in sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int sockaddr_len = sizeof(sockaddr);

	// Create socket with IPv4, UDP connection, and IP protocol
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
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

	// Struct to store client address info
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	// Open file to recieve on server side
	int fd = open(FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if(fd < 0){
		perror("open");
		close(sockfd);
		exit(1);
	}

	// Set up struct to recieve packets
	struct Packet pkt;

	// Set up acknowledgement packet
	struct Packet ack;
	const char* ack_msg = "Acknowledgement";
	ack.len = strlen(ack_msg);
	memcpy(ack.buf, ack_msg, ack.len);
	
	// Packet counter
	int packet_num = 0;

	printf("Server listening on Port %d\n", port);

	while(1) {

		// Receive packets from client
		ssize_t received = recvfrom(sockfd, &pkt, sizeof(pkt), 0, (struct sockaddr *)&client_addr, &client_len);
		if (received < 0) {
			perror("recvfrom failed\n");
			break;
		}
	
		// Empty packet signals end of file transfer
		if (pkt.len == 0){
			printf("End of file transfer\n");
			break;
		}

		//Print recieved packets
		printf("Received packet #%d: len=%d, data=\"", ++packet_num, pkt.len);
		fwrite(pkt.buf, 1, pkt.len, stdout);
		printf("\"\n");

		// Write packets to file
		ssize_t written = write(fd, pkt.buf, pkt.len);
		if(written != pkt.len){
			perror("Write");
			break;
		}

		// Send ack packet
		printf("Sending Ack\n");
		sendto(sockfd, &ack, sizeof(int) + ack.len, 0, (struct sockaddr *)&client_addr, client_len);
	}

	// Close file descriptors
	close(fd);
	close(sockfd);		
	printf("File saved to %s", FILE_NAME);

	return 0;
}

