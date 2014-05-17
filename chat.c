#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

const int MAX = 256;

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

void rkeyboard(int sockn)
{
	int a;
	char buffer[MAX];
	memset((void *) &buffer, 0, MAX);
	read(STDIN_FILENO, buffer, MAX - 1);
	a = write(sockn, buffer, strlen(buffer));
	if (a < 0) error("ERROR writing to socket");
}

void rreceived(int sockn)
{
	int a;
	char buffer[MAX];
	memset((void *) &buffer, 0, MAX);
	a = read(sockn, buffer, MAX - 1);
	if (a == 0) error("Closing socket");
	if (a < 0) error("ERROR reading from socket");
	printf("\rOne:> %s", buffer);
	printf("You:> ");
}
int client(char *ipchar, char *portchar)
{
	int sockfd, portno;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	portno = atoi(portchar);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	server = gethostbyname(ipchar);
	if (server == NULL) {
		fprintf(stderr, "ERROR, no such host\n");
		exit(0);
	}
	memset((void*) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy((void *) &serv_addr.sin_addr.s_addr, (void *) server->h_addr, server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR connecting");
	return sockfd;
}

int server(char *portchar)
{
	int portno;
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen;
	int sockfd, newsockfd;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	memset((void *) &serv_addr, 0, sizeof(serv_addr));
	portno = atoi(portchar);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR on binding");
	listen(sockfd, 5);
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
	if (newsockfd < 0)
		error("ERROR on accept");
	close(sockfd);
	return newsockfd;
}

int main(int argc, char *argv[])
{
	int newsockfd;
	int n;
	fd_set reading;
	if (argc == 3) {
		newsockfd = client(argv[1], argv[2]);
	}
	if (argc == 2) {
		newsockfd = server(argv[1]);
	}
	if (argc < 2) {
		fprintf(stderr, "Usage:\n%s <port_number>\t\t\t-\tfor server\n%s <server_IP> <port_number>\t-\tfor client\n", argv[0], argv[0]);
		exit(1);
	}
	printf("You:> ");

	while (1) {
		FD_ZERO(&reading);
		FD_SET(STDIN_FILENO, &reading);
		FD_SET(newsockfd, &reading);

		n = select(newsockfd + 1, &reading, NULL, NULL, NULL);

		if (n == -1)
			error("ERROR selecting");
		else if (FD_ISSET(0, &reading)) {
			rkeyboard(newsockfd);
		} else if (FD_ISSET(newsockfd, &reading)) {
			rreceived(newsockfd);
		} else if (n == 0){
			printf("ERROR connection time out\n");
			exit(1);
		}
	}

	close(newsockfd);
	return 0;
}

