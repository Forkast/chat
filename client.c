#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

const int MAX = 256;

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

void rkeyboard(int sockn)
{
	char buffer[MAX];
	memset((void*) &buffer, 0, MAX);
	read(0, buffer, MAX - 1);
	int a = write(sockn, buffer, strlen(buffer));
	if (a < 0)
		error("ERROR writing to socket");
}

int max( int a, int b)
{
	return a > b ? a : b;
}
void rreceived(int sockn)
{
	char buffer[MAX];
	memset((void*) &buffer, 0, MAX);
	int a = read(sockn, buffer, MAX - 1);
	if (a == 0)
		error("USER HAS LEFT (NOBODY LIKES YOU)");
	if (a < 0)
		error("ERROR reading from socket");
	printf("\rOne:> %s", buffer);
}

int main(int argc, char *argv[])
{
	int sockfd, portno;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	fd_set reading;
	if (argc < 3) {
	   fprintf(stderr,"usage %s hostname port\n", argv[0]);
	   exit(0);
	}
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	server = gethostbyname(argv[1]);
	if (server == NULL) {
		fprintf(stderr, "ERROR, no such host\n");
		exit(0);
	}
	memset((void*) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR connecting");

	printf("\rYou:> ");
	while (1)
	{
		FD_ZERO(&reading);
		FD_SET(STDIN_FILENO, &reading);
		FD_SET(sockfd, &reading);
		int n = select(sockfd + 1, &reading, NULL, NULL, NULL);
		if (n < 0) {
			printf("ERROR select");
		} else if (n == 0) {
			printf("ERROR timeout\n");
			exit(1);
		} else if (FD_ISSET(sockfd, &reading)) {
			rreceived(sockfd);
		}
			else if (FD_ISSET(STDIN_FILENO, &reading) {
			rkeyboard(sockfd);
		}
	}
	close(sockfd);
	return 0;
}
