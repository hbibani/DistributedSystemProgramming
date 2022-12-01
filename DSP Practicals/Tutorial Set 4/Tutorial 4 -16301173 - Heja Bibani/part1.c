#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<fcntl.h>
#include<signal.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUF_LEN 20000
#define PORT 80

int main(int argc, char* argv[])
{

	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s ser\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int sock_id;
	struct sockaddr_in ser;
	struct hostent* host;
	short serport;
	char req_buffer[100];
	char buffer[2000];
	char header[BUF_LEN];      /*for header of http-request*/
	char delimeter[2] = "\n";

	/*we retrieve a socket_id similar to that of an fd*/
	sock_id = socket(PF_INET, SOCK_STREAM, 0);

	if (sock_id < 0)
	{
		perror("While calling socket()");
		exit(EXIT_FAILURE);
	}

	/*The gethostbyname() function returns a structure of type hostent the given host name.*/
	host = gethostbyname(argv[1]);

	if (host == NULL)
	{
		herror("While calling gethostbyname()");
		exit(EXIT_FAILURE);
	}

	/*get the server protocol for struct sockaddr, ipv4 protocol*/
	ser.sin_family = AF_INET;

	/*hostent was retruned by gethostbyname that took ip address gets information*/
	memcpy(&ser.sin_addr.s_addr, host->h_addr_list[0], host->h_length);

	/*htons converts port into network readable address*/
	ser.sin_port = htons(PORT);

	/*using connect function we can use sock_id to write and read messages to sock_id*/
	if (connect(sock_id, (struct sockaddr*)&ser, sizeof(ser)) != 0)
	{
		perror("While calling connect()");
		exit(EXIT_FAILURE);
	}

	/*prepare HEAD request and send to server*/
	sprintf(req_buffer, "HEAD / HTTP/1.0\r\nHost:%s\r\n\r\n", argv[1]);
	printf("Request Sent: %s", req_buffer);
	if (write(sock_id, req_buffer, strlen(req_buffer)) < 0)
	{
		perror("While writing request");
		close(sock_id);
		exit(EXIT_FAILURE);
	}

	/*declare variables for counting buffer sizes to see if it has been exceeded*/
	int count = 0;
	int readcount = 0;
	/*first get the header message*/
	while ((readcount = read(sock_id, buffer, 1000)) > 0)
	{
		//see if buffer size is exceeded
		count = count + readcount;
		if (count > BUF_LEN)
		{
			fprintf(stderr, "\n Buffer size exceeded. \n");
			close(sock_id);
			exit(EXIT_FAILURE);
		}

		/*form the header string*/
		buffer[readcount + 1] = '\0';
		strcat(header, buffer);
		buffer[0] = '\0';
	}

	/*get first line of the header message which is the http status*/
	char* line = strtok(header, delimeter);

	/*print header status*/
	if (strstr(line, "HTTP"))
	{
		printf("%s\n", line);
	}
	else
	{
		close(sock_id);
		fprintf(stderr, "\n Request reply not correct format. \n");
		exit(EXIT_FAILURE);
	}

	/*walk through other tokens*/
	while (line != NULL)
	{
		/* Content type */
		if (strstr(line, "Content-Type") != NULL)
		{
			printf("%s\n", line);
		}

		/*Last-Modified*/
		if (strstr(line, "Last-Modified") != NULL)
		{
			printf("%s\n", line);
		}

		line = strtok(NULL, delimeter);
	}


	/*close socket*/
	close(sock_id);
	fprintf(stdout, "\nExiting program....\n");
}
