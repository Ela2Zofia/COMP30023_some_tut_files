// A simple server program
// Receives numbers, calculates sum and send result back to client
// Written scrappily, so not the best when it comes to modularity

#define _POSIX_C_SOURCE 200112L
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT "7000"

int main(int argc, char** argv) {
	int sockfd, newsockfd, re;
	struct addrinfo hints, *res;
	struct sockaddr_storage client_addr;
	socklen_t client_addr_size;

	// Create address we're going to listen on (with given port number)
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;       // IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_flags = AI_PASSIVE;     // for bind, listen, accept
	// node (NULL means any interface), service (port), hints, res
	getaddrinfo(NULL, PORT, &hints, &res);

	// Create socket
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockfd < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	// Reuse port if possible
	re = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &re, sizeof(int)) < 0) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	// Bind address to the socket
	if (bind(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	}

	// Listen on socket - means we're ready to accept connections,
	// incoming connection requests will be queued, man 3 listen
	if (listen(sockfd, 5) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	// Accept a connection - blocks until a connection is ready to be accepted
	// Get back a new file descriptor to communicate on
	client_addr_size = sizeof client_addr;
	newsockfd =
		accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_size);
	if (newsockfd < 0) {
		perror("accept");
		exit(EXIT_FAILURE);
	}

	// n is number of characters read
	int n;
	char buffer[256];
	n = read(newsockfd, buffer, 255);
	if (n < 0) {
		perror("read");
		exit(EXIT_FAILURE);
	}
	if (n < 4) {
		fprintf(stderr, "Bad request from server\n");
		exit(EXIT_FAILURE);
	}

	// Number of numberse expected
	uint32_t number_of_numbers = ntohl(*(uint32_t*)&buffer);
	if (n < number_of_numbers * 4 + 4) {
		fprintf(stderr, "Haven't received enough numbers (really?) - "
						"Rerun if you are completing 2.2.1\n");
		exit(EXIT_FAILURE);
	}
	// Calculate sum
	uint32_t sum = 0;
	for (int i = 0; i < number_of_numbers; i++) {
		uint32_t num = ntohl(*(uint32_t*)&buffer[4 + i * 4]);
		sum += num;
	}

	// Send result
	// For convenience, use second buffer...
	unsigned char buffer2[8] = {0,
								0,
								0,
								1,
								sum >> 24,
								(sum & 0xFF0000) >> 16,
								(sum & 0xFF00) >> 8,
								sum & 0xFF};
	printf("Result: %u\n", sum);
	// sleep(10);
	n = write(newsockfd, buffer2, 8);
	if (n < 0) {
		perror("write");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(res);
	close(sockfd);
	close(newsockfd);
	return 0;
}
