/*
* Name: Riley Heike
* 
* Date: April 25th, 2025
* 
* Title: Lab 5 - Part 2: Client Program
* 
* Description: This program creates a client that connects to a server through UDP rdt 3.0 protocol and transfers a file
* 		through packets, and receives acks from server through stop-and-wait
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
#include <time.h>

#define BUF_SIZE 10
#define TIMEOUT 1

typedef struct {
	int seq_ack;
	int len;
	int checksum;
} Header;

typedef struct {
	Header header;
	char data[BUF_SIZE];
} Packet;

// Calculate checksum function (same as server)
int calc_checksum(Packet *pkt){
	int sum = 0;
	char *ptr = (char *)pkt;
	char *end = ptr + sizeof(Header) + pkt->header.len;
	while(ptr < end) sum ^= *ptr++;
	return sum;
}

// Must receive ACK from server before transmitting next packet
int await_ack(int sockfd, Packet *ack, int seq, struct sockaddr_in *server_addr, socklen_t addr_len){
	fd_set readfds;
	struct timeval tv;

	FD_ZERO(&readfds);
	FD_SET(sockfd, &readfds);

	// Timeout setting 
	tv.tv_sec = TIMEOUT;
	tv.tv_usec = 0;

	int rv = select(sockfd + 1, &readfds, NULL, NULL, &tv);

	if(rv == 0){
		printf("Timeout - No ACK.\n");
		return 0;
	}

	// Received ack
	if (recvfrom(sockfd, ack, sizeof(Packet), 0, (struct sockaddr *)server_addr, &addr_len) < 0){
		perror("recvfrom");
		return 0;
	}

	// Check ACK details
	int recv_checksum = ack->header.checksum;
	ack->header.checksum = 0;
	int exp_checksum = calc_checksum(ack);

	// Error handling 
	if(recv_checksum != exp_checksum){
		printf("ACK checksum mismatch: got %d, expected %d\n", recv_checksum, exp_checksum);
		return 0;
	}

	if(ack->header.seq_ack != seq){
		printf("Invalid/corrupt ACK received. Expected %d, got %d\n", seq, ack->header.seq_ack);
		return 0;
	}

	// Valid ACK, continue to next packet
	printf("Valid ACK received: %d\n", ack->header.seq_ack);
	return 1;

}

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
	Packet pkt, ack;
	ssize_t bytes_read;
	int seq = 0;

	// While loop to read from file in chunks of size(packet_len)
	while((bytes_read = read(fd, pkt.data, BUF_SIZE)) > 0){
		// Prepare packets for transfer
		pkt.header.seq_ack = seq;
		pkt.header.len = bytes_read;
		pkt.header.checksum = 0;
		pkt.header.checksum = calc_checksum(&pkt);

		int ack_received = 0;
		while(!ack_received){
			// Send packet, and await ACK
			printf("Sending Packet: seq: %d, len: %ld, checksum: %d\n", seq, bytes_read, pkt.header.checksum);
			sendto(sockfd, &pkt, sizeof(Header) + bytes_read, 0, (struct sockaddr *)&server_addr, addr_len);       	
			ack_received = await_ack(sockfd, &ack, seq, &server_addr, addr_len);
		
		}

		seq = 1 - seq;

	}
	
	// Send empty packet to signify end of file transfer
	pkt.header.seq_ack = 0;
	pkt.header.len = 0;
	pkt.header.checksum = 0;
	pkt.header.checksum = calc_checksum(&pkt);

	// Attempt EOF signal 3 times
	int attempts = 0;
	while (attempts < 3){
		printf("Attempting to signal EOF (try %d)...\n", attempts+1);
		sendto(sockfd, &pkt, sizeof(Header), 0, (struct sockaddr *)&server_addr, addr_len);
		if (await_ack(sockfd, &ack, seq, &server_addr, addr_len)) break;
		attempts++;	
	}

	// Close file descriptors
	close(fd);
	close(sockfd);

	printf("File transfer complete");

	return 0;

}
