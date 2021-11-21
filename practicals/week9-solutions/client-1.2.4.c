// A simple client program for server.c
// To compile: gcc client.c -o client
// To run: start the server, then the client

#define _POSIX_C_SOURCE 200112L
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <unistd.h>

int main(int argc, char** argv) {
	int sockfd;
	struct addrinfo hints, *servinfo, *rp;

	if (argc < 4) {
		fprintf(stderr, "usage %s hostname port path_to_file\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	// Create address
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	// Get addrinfo of server. From man page:
	// The getaddrinfo() function combines the functionality provided by the
	// gethostbyname(3) and getservbyname(3) functions into a single interface
	if ((getaddrinfo(argv[1], argv[2], &hints, &servinfo)) < 0) {
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
	if (rp == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		exit(EXIT_FAILURE);
	}

	// directly copy data from the file to the socket
	int filefd = open(argv[3], O_RDONLY);
	int re = 0;
	// loop until all bytes are sent
	do {
		re = sendfile(sockfd, filefd, NULL, 2048);
	} while (re > 0);
	if (re < 0) {
		perror("sendfile");
		exit(EXIT_FAILURE);
	}

	close(filefd);
	close(sockfd);
	freeaddrinfo(servinfo);

	return 0;
}
