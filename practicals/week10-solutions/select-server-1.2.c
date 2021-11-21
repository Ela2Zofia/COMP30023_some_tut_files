// select-server-1.2.c -- a cheezy multiperson chat server (modified)

#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
	if (argc < 3) {
		fprintf(stderr, "usage: %s ip port\n", argv[0]);
		return 0;
	}

	struct addrinfo hints, *res;
	// create address we're going to listen on (with given port number)
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;       // IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_flags = AI_PASSIVE;     // for bind, listen, accept
	// node (NULL means any interface), service (port), hints, res
	getaddrinfo(argv[1], argv[2], &hints, &res);

	// create socket
	int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockfd < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	// reuse the socket if possible
	int const reuse = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	// bind address to socket
	if (bind(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	}

	// listen on the socket
	listen(sockfd, 5);

	// initialise an active file descriptors set
	fd_set masterfds;
	FD_ZERO(&masterfds);
	FD_SET(sockfd, &masterfds);
	// record the maximum socket number
	int maxfd = sockfd;

	while (1) {
		// monitor file descriptors
		fd_set readfds = masterfds;
		if (select(FD_SETSIZE, &readfds, NULL, NULL, NULL) < 0) {
			perror("select");
			exit(EXIT_FAILURE);
		}

		// loop all possible descriptor
		for (int i = 0; i <= maxfd; ++i)
			// determine if the current file descriptor is active
			if (FD_ISSET(i, &readfds)) {
				// create new socket if there is new incoming connection request
				if (i == sockfd) {
					struct sockaddr_in cliaddr;
					socklen_t clilen = sizeof(cliaddr);
					int newsockfd =
						accept(sockfd, (struct sockaddr*)&cliaddr, &clilen);
					if (newsockfd < 0)
						perror("accept");
					else {
						// add the socket to the set
						FD_SET(newsockfd, &masterfds);
						// update the maximum tracker
						if (newsockfd > maxfd)
							maxfd = newsockfd;
						// print out the IP and the socket number
						char ip[INET_ADDRSTRLEN];
						printf("new connection from %s on socket %d\n",
							   // convert to human readable string
							   inet_ntop(cliaddr.sin_family, &cliaddr.sin_addr,
										 ip, INET_ADDRSTRLEN),
							   newsockfd);
					}
				}
				// a message is sent from the client
				else {
					char buff[256];
					int n = read(i, buff, 256);
					if (n <= 0) {
						if (n < 0)
							perror("read");
						else
							printf("socket %d close the connection\n", i);
						close(i);
						FD_CLR(i, &masterfds);
					}
					// write back to the client
					else if (write(i, buff, n) < 0) {
						perror("write");
						close(i);
						FD_CLR(i, &masterfds);
					}
				}
			}
	}

	return 0;
}
