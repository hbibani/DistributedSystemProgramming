
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>

#define BUF_LEN 48

// This function receives text and shift and 
// returns the encrypted text [caesar cypher with key 10]
int encrypt(char *message, char* encrypt)
{
    int key = 10;
    char* result = (char *)malloc(strlen(message)*sizeof(char));
    char ch;
    int i;
    for(i = 0; message[i] != '\0'; ++i)
    {
		ch = message[i];
		
		if(ch >= 'a' && ch <= 'z')
		{
			ch = ch + key;
			
			if(ch > 'z')
			{
				ch = ch - 'z' + 'a' - 1;
			}
			
			result[i] = ch;
		}
		else if(ch >= 'A' && ch <= 'Z')
		{
			ch = ch + key;
			
			if(ch > 'Z')
			{
				ch = ch - 'Z' + 'A' - 1;
			}
			
			result[i] = ch;
		}
    }

    strcpy(encrypt, result);
    return strlen(result);
}

int main(int argc, char*argv[])
{
	int sock_id;
	
	//structures sock address
	struct sockaddr_in ser;
	struct sockaddr_in cli;
	
	int clilength;
	short port;
	int max_iterations;
	int out, in;
	
	int recv_cnt, i;
	
	//strings in and out use
	char clistring[BUF_LEN];
	char serstring[BUF_LEN];
	
	int ret;
	
	if(argc != 3)
	{
		fprintf(stderr, "Usage: %s Port max_iterations\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	port = atoi(argv[1]); 			//get port number
	max_iterations = atoi(argv[2]); //get max iterations
	
	
	
	/*
		we retrieve a socket_id similar to that of an fd
		specify the domain, type, protocol [usually 0]
		the sock_DGRAM gives the type of  protocol used connectionless, unreliable [udp]
	*/
	sock_id = socket(PF_INET, SOCK_DGRAM, 17);
	
	if(sock_id < 0 )
	{
		perror("While calling socket()");
		exit(EXIT_FAILURE);
	}
	
	/*
		The htonl() function converts the unsigned integer hostlong from host byte order to network byte order.
		The htons() function converts the unsigned short integer hostshort from host byte order to network byte order.
	*/
	ser.sin_family = AF_INET; //ipv4 protocol
	ser.sin_addr.s_addr = htonl(INADDR_ANY);
	ser.sin_port = htons(port);
	
	//bind the socket structure to the system [assign a number]
	ret = bind(sock_id, (struct sockaddr*) &ser, sizeof(ser));
	
	if(ret<0)
	{
		perror("While calling bind()");
		exit(EXIT_FAILURE);
	}
	
	
	for(i = 0 ; i < max_iterations; i++)
	{
		fprintf(stdout, "%d/%d Awaiting messages..\n", i+1, max_iterations);
		clilength = sizeof(cli);
		
		//blocks until something has been recieved
		//gets clients socket information, we use this information for the sendto function
		in=recvfrom(sock_id, clistring, BUF_LEN, 0, (struct sockaddr *)&cli,(socklen_t *)&clilength);
		
		if(in<0)
		{
			perror("While calling recvfrom()");
			exit(EXIT_FAILURE);
		}
		
		//print out recieved value
		fprintf(stdout, "Recieved: %d bytes long\n", in);
		fprintf(stdout, "Received[string]: is \"%s\"\n", clistring);
		
		//encrypt the string
		recv_cnt = encrypt(clistring,serstring);
		fprintf(stdout,"Encrypted string is %d bytes long\n", recv_cnt);
		fprintf(stdout,"Ecrypted string is \"%s\"\n", serstring);
		
		//send the encrypted string using the clients sockaddress retrieved from recvfrom
		out = sendto(sock_id, serstring, recv_cnt+1, 0, (struct sockaddr *)&cli, sizeof(cli));
		
		if(out<0)
		{
			perror("While calling sendto()");
			exit(EXIT_FAILURE);
		}
		
		fprintf(stdout,"Client request now seviced reply sent.\n");
	}
	
	//close socket
	close(sock_id);
	fprintf(stdout,"Completed Iterations, shutting down.\n");
	return 0;	
}



