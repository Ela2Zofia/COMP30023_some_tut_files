// A simple server in the internet domain using TCP
// The port number is passed as an argument
// To compile: gcc server.c -o server
// Reference: Beej's networking guide

#define _POSIX_C_SOURCE 200112L
#include <ctype.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char** argv) {
	int sockfd, newsockfd, n, re, i;
	char buffer[256];
	struct addrinfo hints, *res;
	struct sockaddr_storage client_addr;
	socklen_t client_addr_size;

	if (argc < 2) {
		fprintf(stderr, "ERROR, no port provided\n");
		exit(EXIT_FAILURE);
	}

	// Create address we're going to listen on (with given port number)
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;       // IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_flags = AI_PASSIVE;     // for bind, listen, accept
	// node (NULL means any interface), service (port), hints, res
	getaddrinfo(NULL, argv[1], &hints, &res);

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

	// Read characters from the connection, then process
	while (1) {
		memset(buffer, 0, 256);
		n = read(newsockfd, buffer, 255); // n is number of characters read
		if (n < 0) {
			perror("ERROR reading from socket");
			exit(EXIT_FAILURE);
		}

		// Disconnect
		if (n == 0) {
			break;
		}

		// Exit on Goodbye
		if (strncmp(buffer, "GOODBYE-CLOSE-TCP", 17) == 0) {
			break;
		}

		// Convert to uppercase
		for (i = 0; i < n; i++) {
			buffer[i] = toupper(buffer[i]);
		}

		// Write message back (Note: memset is setting the trailing \0)
		// A rather ugly solution for the buffer
		memmove(buffer + 30, buffer, n);
		memmove(buffer, "Here is the message in upper: ", 30);
		printf("%s\n", buffer);
		n = write(newsockfd, buffer, n + 30);
		if (n < 0) {
			perror("write");
			exit(EXIT_FAILURE);
		}
	}

	freeaddrinfo(res);
	close(sockfd);
	close(newsockfd);
	return 0;
}
