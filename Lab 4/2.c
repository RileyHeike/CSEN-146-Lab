/*
* Name: Riley Heike
* 
* Date: April 25th, 2025
* 
* Title: Lab 4 - Part 2: Client Program
* 
* Description: This program creates a client that connects to a server through UDP socket and transfers a file
* 		through packets, and receives acks from server
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

#define BUF_SIZE 10

struct Packet {
	int len;
	char buf[BUF_SIZE];
};

int main(int argc, char* argv[]){

	if (argc != 4){
		fprintf(stderr, "usage: %s IPAddress PortNo src\n", argv[0]);
		exit(1);
	}

	// Translate args to variables
	const char* server_ip = argv[1];
	int port = atoi(argv[2]);
	const char* src_filename = argv[3];

	// Create socket
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0){
		perror("Socket Creation Failed");
		exit(1);
	}

	// Open source file for reading
	int fd = open(src_filename, O_RDONLY);
	if (fd < 0){
		perror("open");
		close(sockfd);
		exit(1);
	}

	// Fill in server address params
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	socklen_t addr_len = sizeof(server_addr);
	
	// Ensure valid address
	if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0){
		perror("Invalid address");
		exit(1);
	}

	// Set up packet and ack, track bytes read and packet number
	struct Packet pkt;
	struct Packet ack;
	ssize_t bytes_read;
	int packet_num = 0;

	// While loop to read from file in chunks of size(packet_len)
	while((bytes_read = read(fd, pkt.buf, BUF_SIZE)) > 0){
		pkt.len = bytes_read;

		// Send packets
		printf("Sending Packet #%d\n", ++packet_num); 
		if (sendto(sockfd, &pkt, sizeof(int) + pkt.len, 0, (struct sockaddr*)&server_addr, addr_len) < 0){
			perror("sendto");
			break;
		}

		// Receive ack paacket
		ssize_t ack_len = recvfrom(sockfd, &ack, sizeof(ack), 0, (struct sockaddr*)&server_addr, &addr_len);
		if (ack_len > 0){
			printf("Received Packet: \"%.*s\"\n", ack.len, ack.buf);
		}

	}
	
	// Send empty packet to signify end of file transfer
	pkt.len = 0;
	if (sendto(sockfd, &pkt, sizeof(pkt), 0, (struct sockaddr*)&server_addr, addr_len) < 0){
		perror("sendto EOF");
	}

	printf("File transfer complete\n");

	// Close file descriptors
	close(fd);
	close(sockfd);
	return 0;

}
