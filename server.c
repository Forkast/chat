/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

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
	printf("\rOne:> %s", buffer);
	if (a == 0) error("Closing socket");
	if (a < 0) error("ERROR reading from socket");
	printf("You:> ");
}

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	fd_set reading;
	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	memset((void *) &serv_addr, 0, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
		error("ERROR on binding");
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);
	if (newsockfd < 0)
		error("ERROR on accept");

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
	close(sockfd);
	return 0;
}

