#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<string.h>
#include<signal.h>
#include<errno.h>
#include<ctype.h>
#include <sys/wait.h>
#include <unistd.h>
#include<time.h>

/* port number for the server to use, see note above for setting it */
#define SERVER_TCP_PORT 3000

/* These are used in manage_connection(), the numbers aren't critical or
   magic they can be set to anything, but BUF_LEN must be greater than
   COM_BUF_LEN
*/
#define BUF_LEN 200000
#define COM_BUF_LEN 32

/* child manages connections and executes this code */
void manage_connection(int, int);
/*child recieves string and alters it and sends it back to client*/
int server_processing(char* out_string);
/*kill children with signal handler function*/
void handle_sigcld(int);


int main()
{
	int rsock_id;       /* socket descriptor for rendezvous socket */
	int esock_id;       /* socket descriptor for each connected client */
	int err_code; 		/* Error Code holder */
	int clength;
	int pid;        	/* pid of created child */
	struct sockaddr_in server, client;
	struct hostent* cltdet;
	struct sigaction child_sig;


	fprintf(stderr, "M: The Server is starting up...\n");

	/* Set up sigaction for children handling */
	child_sig.sa_handler = handle_sigcld;
	sigfillset(&child_sig.sa_mask);
	child_sig.sa_flags = SA_RESTART | SA_NOCLDSTOP;
	sigaction(SIGCHLD, &child_sig, NULL);


	/* create stream socket TCP */
	rsock_id = socket(PF_INET, SOCK_STREAM, 0);
	if (rsock_id < 0)
	{
		perror("M: rendezvous socket");
		exit(EXIT_FAILURE);
	}

	fprintf(stderr, "M:\tsocket_id=%d\n", rsock_id);


	/* setup address for socket structure then bind */
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(SERVER_TCP_PORT);

	/* bind the socket */
	if ((err_code = bind(rsock_id, (struct sockaddr*)&server, sizeof(server))) < 0)
	{
		perror("M: Binding");
		exit(EXIT_FAILURE);
	}

	fprintf(stderr, "M:\tbound\n");
	/* limit to 5 listen activate connection */
	if ((err_code = listen(rsock_id, 5)) < 0)
	{
		perror("M: Listen error");
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "M:\tlistening\n");
	fprintf(stdout, "M: ...ready to accept connections\n");

	while (1)
	{
		/* server loop created, setup to accet sockets */
		clength = sizeof(client);
		if ((esock_id = accept(rsock_id, (struct sockaddr*)&client,
			(socklen_t*)&clength)) < 0)
		{
			perror("M: connetion error");
			exit(EXIT_FAILURE);
		}

		/* get the name of the client so we can print it */
		cltdet = gethostbyaddr((void*)&client.sin_addr.s_addr, 4, AF_INET);

		if (cltdet == NULL)
		{
			herror("M: resolving the clients address error");
			exit(EXIT_FAILURE);
		}
		fprintf(stderr, "M: Accepted a connection from %s on port %d with esock_id=%d\n",
			cltdet->h_name, ntohs(client.sin_port), esock_id);

		/* child handles connection */
		if ((pid = fork()) == 0)
		{
			srand(time(NULL) ^ (getpid() << 16));
			/* close socket then manage the connection, then exit */
			close(rsock_id);
			manage_connection(esock_id, esock_id);
			exit(EXIT_SUCCESS);
		}
		else
		{
			/* Parent specifies which child handles connection */
			close(esock_id);
			fprintf(stderr, "M: process %d will service this.\n", pid);
		}
	}
	/*close the socket */
	close(rsock_id);
}



/*child recieves string and alters it and sends it back to client*/
int server_processing(char* out_string)
{
	/*create strings and other parameters*/
	char* storage;
	char  response[BUF_LEN];
	int  num_lines;
	char *character;
	char *following;
	char *firstline;
	/*create server string*/
	char lemons[] = "Gay go up, and gay go down,\n" \
		      "To ring the bells of London town. \n\n" \
		      "Bull's eyes and targets,\n" \
		      "Say the bells of St. Margret's.\n\n" \
		      "Brickbats and tiles,\n" \
		      "Say the bells of St. Giles'.\n\n" \
		      "Halfpence and farthings, \n" \
		      "Say the bells of St. Martin's.\n\n" \
		      "Oranges and lemons,\n" \
		      "Say the bells of St. Clement's.\n\n" \
		      "Pancakes and fritters,\n" \
		      "Say the bells of St. Peter's.\n\n" \
		      "Two sticks and an apple,\n" \
		      "Say the bells at Whitechapel.\n\n" \
		      "Pokers and tongs,\n" \
		      "Say the bells at St. John's.\n\n" \
		      "Kettles and pans,\n" \
		      "Say the bells at St. Ann's.\n\n" \
		      "Old Father Baldpate, \n" \
		      "Say the slow bells at Aldgate.\n\n" \
		      "Maids in white Aprons\n" \
		      "Say the bells of St Catherine's.\n\n" \
		      "You owe me ten shillings,\n" \
		      "Say the bells of St. Helen's.\n\n" \
		      "When will you pay me?\n" \
		      "Say the bells at Old Bailey.\n\n" \
		      "When I grow rich,\n" \
		      "Say the bells at Shoreditch.\n\n" \
		      "Pray when will that be?\n" \
		      "Say the bells of Stepney.\n\n" \
		      "I'm sure I don't know,\n" \
		      "Says the great bell at Bow.\n\n" \
		      "Here comes a candle to light you to bed,\n" \
		      "And here comes a chopper to chop off your head.\n";
		      
	storage = lemons;
	storage[strlen(storage)+1] = '\0';
	
	num_lines = rand() % 33;
	/*get first line*/
	firstline = strtok(storage, "\n");
	strcat(response, firstline);
	strcat(response, "\n");
	
	/*produce the response with random number of lines*/
	for (int i = 0; i < num_lines; i++)
	{
		following = strtok(NULL, "\n");
		strcat(response, following);
		strcat(response, "\n");
		if (i % 2 == 0)
		{
			strcat(response, "\n");
		}
	}
	response[strlen(response) + 1] = '\0';
	//fprintf( stdout ,"\ns4 %s\n", response);
	strcpy(out_string, response);

	return 1;
}

/* child manages connections and executes this code */
void manage_connection(int in, int out)
{
	/* read count and buffer count */
	int readcount, count, recv_count;
	/* string buffers for various requirments */
	char input_buffer[BUF_LEN];
	char inbuf[BUF_LEN];
	char* firstline;
	char response[BUF_LEN];
	int string_length;
	char h_sendout[BUF_LEN + BUF_LEN];
	int test = 0;
	char* request;
	char* host;
	char* following;
	char* teststring;
	
	fprintf(stdout, "\nStarting up\n");
	fprintf(stderr, "\n Read getting done.....\n\n");
	
	/* read in request */
	while (1)
	{
		while (1)
		{
			/*count amount read in*/
			readcount = read(in, input_buffer, 10);
			count = count + readcount;
			
			/*read if count is greater than 0*/
			if (readcount > 0)
			{
				/*check if count exeeded buffer-length*/
				if (count > BUF_LEN)
				{
					fprintf(stderr, "\n Buffer size exceeded. \n");
					close(in);
					exit(EXIT_FAILURE);
				}
				
				/*null terminate string*/
				input_buffer[readcount] = '\0';
				strcat(inbuf, input_buffer);
			}
			
			/*break at end*/
			if (readcount < 10)
			{
				break;
			}
		}

		/*check for terminator to exit loop*/
		if (strstr(inbuf, "\r\n\r\n") != NULL)
			break;
	}

	/*cancel if adequate information not present*/
	if (strstr(inbuf, "Host:") == NULL)
	{
		test = 1;
		fprintf(stderr, "\nInvalid request, need host information\n");
	}


	/*prepare string to test for processing*/
	if (test == 0)
	{
		host = strstr(inbuf, "\n");
		host[strlen(host) - 4] = '\0';
		request = strtok(inbuf, "\r");
		printf("Request:\n%s", request);
		printf("%s\n", host);
	}


	if (test == 0)
	{
		
		/*check for to see if request is for the root document*/
		if ((strstr(request, "GET /") != NULL) || (strstr(request, "HEAD /") != NULL))
		{
			teststring = strstr(request, "/");
			if ((strstr(teststring, "/ HTTP/1.0") == NULL) && (strstr(teststring, "/ HTTP/1.1") == NULL))
			{
				test = 1;
				strcpy(h_sendout, "HTTP/1.1 404 Not Found\r\n\r\n");

				if (strstr(teststring, "HTTP/1.0") != NULL)
				{
					strcpy(h_sendout, "HTTP/1.0 404 Not Found\r\n\r\n");
				}
				else
				{
					strcpy(h_sendout, "HTTP/1.1 404 Not Found\r\n\r\n");
				}

				/*write the request to client place into buffer*/
				if (write(out, h_sendout, strlen(h_sendout)) < 0)
				{
					perror("While writing request");
					close(in);
					exit(EXIT_FAILURE);
				}


				fprintf(stdout, "\nChild-process closing down......\n");
				/*close socket*/
				close(in);
				exit(1);
			}
		}


		fprintf(stderr, "\nInvalid request, need host information\n");
		/*Check if http request is HEAD*/
		if (((strstr(request, "HEAD / HTTP/1.0") != NULL) || (strstr(request, "HEAD / HTTP/1.1") != NULL)))
		{
			fprintf(stderr, "\nClient is using HTTP HEAD request\n");
			/*process request and send out*/
			server_processing(response);
			string_length = strlen(response);
			fprintf(stderr, "\nResponse: %s\n", response);

			if (strstr(request, "HTTP/1.0") != NULL)
			{
				sprintf(h_sendout, "HTTP/1.0 200 OK\r\nContent-Type:text/plain\r\nContent-Length:%i\r\n\r\n", string_length);
			}
			else
			{
				sprintf(h_sendout, "HTTP/1.1 200 OK\r\nContent-Type:text/plain\r\nContent-Length:%i\r\n\r\n", string_length);
			}

			/*write the request to client place into buffer*/
			if (write(out, h_sendout, strlen(h_sendout)) < 0)
			{
				perror("While writing request");
				close(in);
				exit(EXIT_FAILURE);
			}
		}
		else if (((strstr(request, "GET / HTTP/1.1") != NULL) || (strstr(request, "GET / HTTP/1.0") != NULL)))
		{
			fprintf(stderr, "\nClient is using HTTP GET request\n");
			/*process request and send out*/
			server_processing(response);
			string_length = strlen(response);
			fprintf(stderr, "\nResponse: %s\n", response);
			response[strlen(response)] = '\0';
			if (strstr(request, "HTTP/1.0") != NULL)
			{
				sprintf(h_sendout, "HTTP/1.0 200 OK\r\nContent-Type:text/plain\r\nContent-Length:%i\r\n\r\n%s", string_length, response);
			}
			else
			{
				sprintf(h_sendout, "HTTP/1.1 200 OK\r\nContent-Type:text/plain\r\nContent-Length:%i\r\n\r\n%s", string_length, response);
			}

			/*write the request to client place into buffer*/
			if (write(out, h_sendout, strlen(h_sendout)) < 0)
			{
				perror("While writing request");
				close(in);
				exit(EXIT_FAILURE);
			}
		}
		else
		{
			strcpy(h_sendout, "HTTP/1.1 501 Not Implemented\r\n\r\n");
			/*write the request to client place into buffer*/
			if (write(out, h_sendout, strlen(h_sendout)) < 0)
			{
				perror("While writing request");
				close(in);
				exit(EXIT_FAILURE);
			}
		}
	}

	if (test == 1)
	{
		strcpy(h_sendout, "HTTP/1.1 501 Not Implemented\r\n\r\n");
		/*write the request to client place into buffer*/
		if (write(out, h_sendout, strlen(h_sendout)) < 0)
		{
			perror("While writing request");
			close(in);
			exit(EXIT_FAILURE);
		}
	}

	fprintf(stdout, "\nChild-process closing down......\n");
	/*close socket*/
	close(in);
}

void handle_sigcld(int signo)
{
	pid_t child;
	/*wait for children and destroy all that is waiting*/
	while (0 > waitpid(-1, NULL, WNOHANG));
}
