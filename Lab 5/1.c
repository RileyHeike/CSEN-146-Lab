/*
* Name: Riley Heike
* 
* Date: April 30th, 2025
* 
* Title: Lab 5 - Part 1: Server Program
* 
* Description: This program recieves a file from a client through UDP rdt 3.0 stop and wait connection
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

typedef struct {
	int seq_ack;
	int len;
	int checksum;
} Header;

typedef struct {
	Header header;
	char data[BUF_SIZE];
} Packet;

// Checksum calculation function
int calc_checksum(Packet *pkt){
	int sum = 0;
	char *ptr = (char *)pkt;
	char *end = ptr + sizeof(Header) + pkt->header.len;
	while(ptr < end) sum ^= *ptr++;
	return sum;
}

// Random probability used to simulate packet loss
int random_result(float loss_prob){
	return ((float)rand() / RAND_MAX) < loss_prob;
}

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
	Packet pkt, ack;
	int seq = 1;
	printf("Server listening on Port %d\n", port);

	while(1) {

		// Receive packets from client
		ssize_t received = recvfrom(sockfd, &pkt, sizeof(pkt), 0, (struct sockaddr *)&client_addr, &client_len);
		if (received < 0) {
			perror("recvfrom failed\n");
			break;
		}
	
		// Empty packet signals end of file trasnfer
		if (pkt.header.len == 0){
			printf("End of file transfer\n");
			
			// Send last ACK
			ack.header.seq_ack = pkt.header.seq_ack;
			ack.header.len = 0;
			ack.header.checksum = 0;
			ack.header.checksum = calc_checksum(&ack);

			sendto(sockfd, &ack, sizeof(Header), 0, (struct sockaddr *)&client_addr, client_len);
			printf("Final ACK %d sent\n", ack.header.seq_ack);
			
			break;
		}

		// Compare checksum to ecpected checksum
		int checksum = pkt.header.checksum;
		pkt.header.checksum = 0;
		int exp_checksum = calc_checksum(&pkt);

		printf("Recieved packet: { header: {seq: %d, len: %d, checksum: %d (exp: %d)}, data: %s }\n", pkt.header.seq_ack, pkt.header.len, checksum, exp_checksum, pkt.data);

		int send_ack = 1;

		// Ensure packet is not corrupt/invalid
		if (checksum == exp_checksum && pkt.header.seq_ack == seq){
			// Write packets to file
			ssize_t written = write(fd, pkt.data, pkt.header.len);
			if(written != pkt.header.len){
				perror("Write");
				break;
			}

			// Prepare ACK packet
			ack.header.seq_ack = seq;
			ack.header.len = 0;
			ack.header.checksum = 0;
			ack.header.checksum = calc_checksum(&ack);
			seq = 1 - seq;
		}
		else {
			// Send previous ACK at receival of invalid packet
			ack.header.seq_ack = 1 - seq;
			ack.header.len = 0;
			ack.header.checksum = 0;
			ack.header.checksum = calc_checksum(&ack);
			printf("Duplicate/corrupt packet. Resending previous ACK: %d\n", ack.header.seq_ack);
		}

		// Simulate packet loss
		if(!random_result(0.2)){
			sendto(sockfd, &ack, sizeof(Header), 0, (struct sockaddr *)&client_addr, client_len);
			printf("ACK %d sent\n", ack.header.seq_ack);
		}
		else{
			printf("Simulated ACK loss. ACK %d not sent.\n", ack.header.seq_ack);
		
		}

	}

	// Close file descriptors
	close(fd);
	close(sockfd);		
	printf("File saved to %s", FILE_NAME);

	return 0;
}

