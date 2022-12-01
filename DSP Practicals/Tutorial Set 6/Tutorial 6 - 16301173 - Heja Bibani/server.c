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
#include <arpa/inet.h>

/* port number for the server to use, see note above for setting it */
#define SERVER_TCP_PORT 9999


/* These are used in manage_connection(), the numbers aren't critical or
   magic they can be set to anything, but BUF_LEN must be greater than
   COM_BUF_LEN
*/
#define BUF_LEN 512
#define COM_BUF_LEN 32

/* child manages connections and executes this code */
void manage_connection(int, int);
/*child recieves string and alters it and sends it back to client*/
void server_processing(char* in_string, char* out_string);
/*kill children with signal handler function*/
void handle_sigcld(int);


int main()
{
	int rsock_id;       /* socket descriptor for rendezvous socket */
	int esock_id;       /* socket descriptor for each connected client */
	int err_code; 		/* Error Code holder */
	int clength;
	int pid;        	/* pid of created child */
	struct sockaddr_in6 server, client;
	struct sigaction child_sig;
	int ret;			/*return value of setsockopt*/
	int flag = 1;		/*setsockopt flag to turn on or off an option*/
	char buf1[200];		/*server address name */
	char buf2[200];		/*server address numeric*/

	fprintf(stderr, "M: The Server is starting up...\n");

	/* Set up sigaction for children handling */
	child_sig.sa_handler = handle_sigcld;
	sigfillset(&child_sig.sa_mask);
	child_sig.sa_flags = SA_RESTART | SA_NOCLDSTOP;
	sigaction(SIGCHLD, &child_sig, NULL);

	/* create stream socket TCP */
	rsock_id = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

	if (rsock_id < 0)
	{
		perror("M: rendezvous socket");
		exit(EXIT_FAILURE);
	}

	fprintf(stderr, "M:\tsocket_id=%d\n", rsock_id);

	/*make address re-usable*/
	ret = setsockopt(rsock_id, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

	if (ret == -1)
	{
		perror("setsockopt()");
		exit(EXIT_FAILURE);
	}

	flag = 0; /*change flag for compatible option*/
	/*Make Ipv4/ipv6 compatible, turn off ipv6 only*/
	ret = setsockopt(rsock_id, IPPROTO_IPV6, IPV6_V6ONLY, &flag, sizeof(flag));

	if (ret == -1)
	{
		perror("setsockopt()");
		exit(EXIT_FAILURE);
	}

	/* setup address for socket structure then bind */
	memset(&server, 0, sizeof(server));
	server.sin6_family = AF_INET6;
	server.sin6_addr = in6addr_any;
	server.sin6_port = htons(SERVER_TCP_PORT);

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

		/*get name information of both forms*/
		if (getnameinfo((struct sockaddr*)&client, sizeof(client), buf1, sizeof(buf1), NULL, 0, NI_NAMEREQD) != 0)
		{
			strcpy(buf1, "???");
		}

		(void)getnameinfo((struct sockaddr*)&client, sizeof(client), buf2, sizeof(buf2), NULL, 0, NI_NUMERICHOST);

		fprintf(stderr, "M: Accepted a connection from %s(%s) on port %d with esock_id=%d\n",
			buf1, buf2, ntohs(client.sin6_port), esock_id);

		/* child handles connection */
		if ((pid = fork()) == 0)
		{
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
void server_processing(char* in_string, char* out_string)
{
	int i, length;
	char c;

	length = strlen(in_string);

	for (i = 0; i < length; i++)
	{
		c = tolower(in_string[i]);
		if (c == in_string[i])
			out_string[i] = toupper(in_string[i]);
		else
			out_string[i] = c;
	}
	out_string[length] = '\0';
}

/* child manages connections and executes this code */
void manage_connection(int in, int out)
{
	/* read count and buffer count */
	int read_count, buffer_count;
	/* string buffers for various requirments */
	char input_buffer[BUF_LEN], output_buffer[1024], recv_buffer[BUF_LEN], hostname[40], inbuf[BUF_LEN];
	/* bit at start of output messages to show which child */
	char pfix[100];
	int i;
	char end_of_data = '&';
	/* get name of server, then print to send out */
	gethostname(hostname, 40);
	/* get server strings prepared, and then print some initial statements to client */
	sprintf(pfix, "\tC %d: ", getpid());
	fprintf(stdout, "\n%sStarting up\n", pfix);
	sprintf(output_buffer, "\n\nExample Server on host: %s, enter 'X' to exit\n", hostname);
	write(out, output_buffer, strlen(output_buffer));
	/*zero strings for preparation*/
	inbuf[0] = '\0';
	output_buffer[0] = '\0';

	while (1)
	{
		/*
			while loop which continues communication with process, continously reading information
			processing that information, sending the information back to client and then requesting
			more information
		*/
		buffer_count = 0;
		while (1)
		{
			/* clear string read into buffer from outside */
			input_buffer[0] = '\0';
			read_count = read(in, input_buffer, BUF_LEN);
			if (read_count > 0)
			{
				/* check buffer size */
				if ((buffer_count + read_count) > BUF_LEN)
				{
					fprintf(stderr, "\n%s Buffer size exceeded!\n", pfix);
					close(in);
					exit(EXIT_FAILURE);
				}

				input_buffer[read_count] = '\0';
				/* concatenate string and send back */
				strcat(inbuf, input_buffer);
				/* readjust and calculate buffer size */
				buffer_count = buffer_count + read_count;
				/* print recieved data */
				fprintf(stdout, "%sRead in [string :: size :: %ld]: %s\n", pfix, strlen(input_buffer), input_buffer);
				if (input_buffer[read_count - 3] == end_of_data)
					break;
			}
			else if (read_count == 0)
			{
				/* check read count for exit status */
				fprintf(stderr, "\n%sClient has closed...closing\n", pfix);
				close(in);
				exit(EXIT_FAILURE);
			}
			else
			{
				/* if error exit */
				sprintf(pfix, "\tC %d: Reading from connection", getpid());
				perror(pfix);
				close(in);
				exit(EXIT_FAILURE);
			}
		}

		/*chop off /r/n, which is the last two characters for telnet connections*/
		inbuf[buffer_count - 2] = '\0';
		/* end connection if X is pressed */
		if (inbuf[0] == 'X')
			break;

		/*  get rid of null termination character and then process data */
		inbuf[strlen(inbuf)] = '\0';
		server_processing(inbuf, recv_buffer);

		/* send it back with a message and next prompt */
		sprintf(output_buffer, "The server received %ld characters, rotated string:\n%s\n\nEnter next string: ", strlen(recv_buffer), recv_buffer);
		write(out, output_buffer, strlen(output_buffer));
		/*clear buffers for safety*/
		inbuf[0] = '\0';
		output_buffer[0] = '\0';
		recv_buffer[0] = '\0';
	}

	fprintf(stderr, "\n%sClient exited, closing down\n", pfix);
	/*close socket*/
	close(in);
}

void handle_sigcld(int signo)
{
	pid_t child;
	/*wait for children and destroy all that is waiting*/
	while (0 > waitpid(-1, NULL, WNOHANG));
}
