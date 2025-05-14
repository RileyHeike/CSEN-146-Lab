/*

 Name: Riley Heike
 
 Date: April 9th, 2025

 Title: Lab2 - Request Time exercise
 
 Description: This program calculates the expected response times of the scenario in exercise 2

*/

#include <stdio.h>
#define FIRST_DNS_SERVER 3
#define SECOND_DNS_SERVER 20
#define THIRD_DNS_SERVER 26
#define RTTHTTP 47

int main(int argc, char* argv[]){
	
	// First server, second server, third server RTT, followed by RTTHTTP
	// to establish connection and retreive object
	int link_to_object = FIRST_DNS_SERVER + SECOND_DNS_SERVER + THIRD_DNS_SERVER + (2 * RTTHTTP);
	
	//Retrieve initial object, and RTTHTTP to re-establish connection and retreive 6 more objects
	int non_persistent = link_to_object + (6 * 2 * RTTHTTP);
	
	//Retreive 6 objects in parallel, and no need to re-establish connection
	int parallel_persistent = link_to_object + RTTHTTP;

	//Retreive 6 objects in parallel, but with re-establishing connection
	int parallel_non_persistent = link_to_object + (2 * RTTHTTP);


	printf("One object: %d ms\n", link_to_object);
	printf("Non-persistent, 6 objects: %d ms\n", non_persistent);
	printf("6 Parallel connection - Persistent: %d ms\n", parallel_persistent);
	printf("6 Parallel connection - Non-persistent: %d ms\n", parallel_non_persistent);
}
