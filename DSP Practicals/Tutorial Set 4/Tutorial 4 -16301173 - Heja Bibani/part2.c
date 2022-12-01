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

#define BUF_LEN 200000
#define PORT 80

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s server request-type\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if (!(strcmp(argv[2], "get") == 0 || strcmp(argv[2], "head") == 0))
	{
		fprintf(stderr, "'get' or 'head' request as second argument\n");
		exit(EXIT_FAILURE);
	}

	int sock_id;
	struct sockaddr_in ser; /*sock address will contain the protocol, port and address via the other inaddr struct*/
	struct hostent* host;   /*used by gethostbyname which returns a struct hostent*/
	short serport;
	char req_buffer[BUF_LEN];
	char buffer[200];
	char header[BUF_LEN];      /*for header of http-request*/
	char bodybuffer[BUF_LEN];  /*for body of http-request*/
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

	/*	IP4 and hostent was retruned by gethostbyname*/
	ser.sin_family = AF_INET;
	memcpy(&ser.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
	/*htons converts port into network readable address*/
	ser.sin_port = htons(PORT);
	/*using connect function we can use sock_id to write and read messages to sock_id*/
	if (connect(sock_id, (struct sockaddr*)&ser, sizeof(ser)) != 0)
	{
		perror("While calling connect()");
		exit(EXIT_FAILURE);
	}


	/*generate request and write to socket depending on request type*/
	if (strcmp(argv[2], "get") == 0)
	{
		sprintf(req_buffer, "GET / HTTP/1.0\r\nHost:%s\r\n\r\n", argv[1]);
	}

	if (strcmp(argv[2], "head") == 0)
	{
		sprintf(req_buffer, "HEAD / HTTP/1.0\r\nHost:%s\r\n\r\n", argv[1]);
	}

	printf("Request Sent: %s", req_buffer);

	/*write the request to website place into buffer*/
	if (write(sock_id, req_buffer, strlen(req_buffer)) < 0)
	{
		perror("While writing request");
		close(sock_id);
		exit(EXIT_FAILURE);
	}

	/*declare variables for counting buffer sizes to see if it has been exceeded*/
	int readcount = 0;
	int count = 0;
	if (strcmp(argv[2], "head") == 0)
	{
		/*get the header message, note: buffer size different from get request*/
		while ((readcount = read(sock_id, buffer, 100)) > 0)
		{

			//see if buffer size is exceeded
			count = count + readcount;
			if (count > BUF_LEN)
			{
				fprintf(stderr, "\n Buffer size exceeded. \n");
				close(sock_id);
				exit(EXIT_FAILURE);
			}

			/*produce header*/
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
			fprintf(stderr, "\n Request reply not in correct format. \n");
			exit(EXIT_FAILURE);
		}

		/* walk through other tokens */
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
	}

	/*print out body or print out HTTP error code*/
	if (strcmp(argv[2], "get") == 0)
	{
		/*first get the header message, size buffer 1 to cleanly seperate header from body*/
		while ((readcount = read(sock_id, buffer, 1)) > 0)
		{

			//see if buffer size is exceeded
			count = count + readcount;
			if (count > BUF_LEN)
			{
				fprintf(stderr, "\n Buffer size exceeded. \n");
				close(sock_id);
				exit(EXIT_FAILURE);
			}

			/*produce header*/
			buffer[readcount + 1] = '\0';
			strcat(header, buffer);
			buffer[0] = '\0';

			/*look for terminator between header and body*/
			if (strstr(header, "\r\n\r\n"))
			{
				break;
			}
		}

		/*get first line of the header message which is the http status*/
		char* line = strtok(header, delimeter);

		/*set status value to determine if body needs to be printed*/
		if (strstr(line, "200") != NULL)
		{
			/*buffer size altered which is more suitable for the body*/
			while ((readcount = read(sock_id, bodybuffer, BUF_LEN - 2)) > 0)
			{
				bodybuffer[readcount + 1] = '\0';
				printf("%s", bodybuffer);
				bodybuffer[0] = '\0';
			}
		}
		else
		{
			/*print first http status line if request was not 200*/
			if (strstr(line, "HTTP"))
			{
				printf("%s\n", line);
			}
			else
			{
				close(sock_id);
				fprintf(stderr, "\n Request reply not in correct format. \n");
				exit(EXIT_FAILURE);
			}
		}
	}

	/*close socket*/
	close(sock_id);
	fprintf(stdout, "\nExiting program....\n");
}
