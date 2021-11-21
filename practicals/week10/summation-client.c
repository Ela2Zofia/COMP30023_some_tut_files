// A simple client program
// Takes numbers as CLI arguments, send to server, and prints response
// Written scrappily, so not the best when it comes to modularity

#define _POSIX_C_SOURCE 200112L
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ADDR "127.0.0.1"
#define PORT "7000"

int main(int argc, char** argv) {
	int sockfd, n;
	struct addrinfo hints, *servinfo, *rp;
	char buffer[256];

	if (argc < 2) {
		fprintf(stderr, "usage %s number1 number2 ...\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	// Create address
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	// Get addrinfo of server. From man page:
	// The getaddrinfo() function combines the functionality provided by the
	// gethostbyname(3) and getservbyname(3) functions into a single interface
	if ((getaddrinfo(ADDR, PORT, &hints, &servinfo)) < 0) {
		perror("getaddrinfo");
		exit(EXIT_FAILURE);
	}

	// Connect to first valid result
	// Why are there multiple results? see man page (search 'several reasons')
	// How to search? enter /, then text to search for, press n/N to navigate
	for (rp = servinfo; rp != NULL; rp = rp->ai_next) {
		sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sockfd == -1)
			continue;

		if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1)
			break; // success

		close(sockfd);
	}
	freeaddrinfo(servinfo);
	if (rp == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		exit(EXIT_FAILURE);
	}

	// Numbers to buffer
	for (int i = 1; i < argc; i++) {
		uint32_t n = atoi(argv[i]);
		if (n < 0) {
			fprintf(stderr, "Bad input\n");
			exit(EXIT_FAILURE);
		}
		uint32_t* val = (uint32_t*)&buffer[(i - 1) * 4];
		*val = htonl(n);
	}

	// Number of numbers to be sent, 4 byte prefix
	uint32_t num_of_numbers = argc - 1;
	char buffer2[4];
	uint32_t* buffer2_cast = (uint32_t*)buffer2;
	*buffer2_cast = htonl(num_of_numbers);

	// Send prefix to server
	n = write(sockfd, buffer2, 4);
	if (n < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	// sleep(2);
	// Send buffer
	n = write(sockfd, buffer, num_of_numbers * 4);
	if (n < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	// Read message from server
	n = read(sockfd, buffer, 255);
	if (n < 0) {
		perror("read");
		exit(EXIT_FAILURE);
	}
	if (n != 8 || buffer[0] != 0 || buffer[1] != 0 || buffer[2] != 0 ||
		buffer[3] != 1) {
		fprintf(stderr, "Bad response from server\n");
		exit(EXIT_FAILURE);
	}
	// Print result
	uint32_t result = ntohl(*(uint32_t*)&buffer[4]);
	printf("The result: %u\n", result);

	close(sockfd);
	return 0;
}
