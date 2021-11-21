/* Client for 1.2 */
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/sendfile.h>
#include <unistd.h>

void process_download(const int sockfd, const char* file_name);
int setup_client_socket(const int port, const char* server_name,
						struct sockaddr_in* serv_addr);
void process_upload(const int sockfd, const char* file_name);

int main(int argc, char* argv[]) {
	struct sockaddr_in serv_addr;
	char* server;
	int port;
	int sockfd;
	char buffer[256];

	if (argc < 3) {
		fprintf(stderr, "usage: %s hostname port\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	port = atoi(argv[2]);
	server = argv[1];

	while (1) {
		/* Make connection */
		sockfd = setup_client_socket(port, server, &serv_addr);
		if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) <
			0) {
			perror("connect");
			exit(EXIT_FAILURE);
		}

		/* Note: fgets stores \0 */
		printf("Enter command: ");
		if (!fgets(buffer, 255, stdin)) {
			break;
		}
		strtok(buffer, "\n");

		/* Branch off depending on given input */
		if (strncmp(buffer, "DOWNLOAD", 8) == 0) {
			if (!(buffer + 9)) {
				fprintf(stderr, "No file name has been given\n");
				close(sockfd);
				continue;
			}
			process_download(sockfd, buffer + 9);
		} else if (strncmp(buffer, "UPLOAD", 6) == 0) {
			if (!(buffer + 7)) {
				fprintf(stderr, "No file name has been given\n");
				close(sockfd);
				continue;
			}
			process_upload(sockfd, buffer + 7);
		} else {
			fprintf(stderr, "Invalid command\n");
		}

		/* Close to let server know that we've finished sending our message */
		close(sockfd);
	}
}

/* Handles DOWNLOAD command */
void process_download(const int sockfd, const char* file_name) {
	int filefd;
	char buffer[2048];
	int n;

	/* Send download command to server */
	sprintf(buffer, "DOWNLOAD %s", file_name);
	n = write(sockfd, buffer, strlen(buffer));
	if (n < 0) {
		perror("write");
		exit(EXIT_FAILURE);
	}

	/* Read initial message */
	n = read(sockfd, buffer, 2047);
	if (n < 0) {
		perror("read");
		exit(EXIT_FAILURE);
	}
	buffer[n] = '\0';

	/* Take action depending on server response */
	if (strncmp(buffer, "NOT-FOUND", 9) == 0) {
		fprintf(stderr, "Server could not find %s\n", file_name);
		return;
	} else if (strncmp(buffer, "OK ", 3) == 0) {
		/* Open file for writing */
		filefd = open(file_name, O_WRONLY | O_CREAT, 0600);

		/* Write initial buffer contents */
		n = write(filefd, buffer + 3, n - 3);
		if (n < 0) {
			perror("write");
			exit(EXIT_FAILURE);
		}
	} else {
		fprintf(stderr, "Wrong response from server, ignoring\n");
		return;
	}

	// Read from socket until whole file is received
	while (1) {
		n = read(sockfd, buffer, 2048);
		if (n == 0) {
			break;
		}
		if (n < 0) {
			perror("read");
			exit(EXIT_FAILURE);
		}

		n = write(filefd, buffer, n);
		if (n < 0) {
			perror("write");
			exit(EXIT_FAILURE);
		}
	}

	printf("Received file %s\n", file_name);
}

/* Handles UPLOAD command */
void process_upload(const int sockfd, const char* file_name) {
	int filefd;
	int n, re;
	char buffer[2048];

	if (access(file_name, F_OK) != -1) {
		/* Open file */
		filefd = open(file_name, O_RDONLY);
		if (!filefd) {
			perror("open");
			exit(EXIT_FAILURE);
		}

		/* Write "UPLOAD " and file name */
		n = sprintf(buffer, "UPLOAD %s ", file_name);
		n = write(sockfd, buffer, n);
		if (n < 0) {
			perror("write");
			exit(EXIT_FAILURE);
		}

		/* Send file contents */
		re = 0;
		do {
			re = sendfile(sockfd, filefd, NULL, 2048);
		} while (re > 0);
		if (re < 0) {
			perror("ERROR sending file");
			exit(EXIT_FAILURE);
		}
	} else {
		fprintf(stderr, "File not found\n");
	}
}

/* Create and return a socket bound to the given port and server */
int setup_client_socket(const int port, const char* server_name,
						struct sockaddr_in* serv_addr) {
	int sockfd;
	struct hostent* server;

	server = gethostbyname(server_name);
	if (!server) {
		fprintf(stderr, "ERROR, no such host\n");
		exit(EXIT_FAILURE);
	}
	bzero((char*)serv_addr, sizeof(serv_addr));
	serv_addr->sin_family = AF_INET;
	bcopy(server->h_addr_list[0], (char*)&serv_addr->sin_addr.s_addr,
		  server->h_length);
	serv_addr->sin_port = htons(port);

	/* Create socket */
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	return sockfd;
}
