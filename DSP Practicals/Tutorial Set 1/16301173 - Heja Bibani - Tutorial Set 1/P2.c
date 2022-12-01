#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>

#define BUF_LEN 48

int main(int argc, char* argv[])
{
	int sock_id;
	//sock address will contain the protocol, port and address via the other inaddr struct
	struct sockaddr_in ser;
	//used by gethostbyname which returns a struct hostent
	struct hostent *host;
	int serlen;
	int strsize;
	short serport;
	int out, in; //the return values of rcvfrom or sendto
	char clientstring[BUF_LEN];
	char serverstring[BUF_LEN];
	
	if(argc !=4)
	{
		fprintf(stderr, "Usage: %s ser Port send_string\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	//The gethostbyname() function returns a structure of type hostent the given host name.
	host = gethostbyname(argv[1]); 
	
	if(host == NULL)
	{
		herror("While calling gethostbyname()");
		exit(EXIT_FAILURE);
	}
	
	serport = atoi(argv[2]); //get the port
	strcpy(clientstring, argv[3]); //get the string
	
	/*
		we retrieve a socket_id similar to that of an fd
		specify the domain, type, protocol [usually 0]
		the sock_DGRAM gives the type of  protocol used connectionless, unreliable [udp]
	*/
	sock_id = socket(PF_INET, SOCK_DGRAM, 0);
	
	if(sock_id<0)
	{
		perror("While calling socket()");
		exit(EXIT_FAILURE);
	}
	
	ser.sin_family = AF_INET; //get the server protocol for struct sockaddr, ipv4 protocol
	
	/*
		hostent was retruned by gethostbyname that took ip address gets information
		hostent has all the neccessary information for the address
		memcpy copies the information from hostent and places it into the sock structure
	*/
	
	memcpy(&ser.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
	//htons converts port into network readable address
	ser.sin_port = htons(serport);
	//to detemrine the size of read and write calls
	strsize = strlen(clientstring) + 1; 
	
	fprintf(stdout,"Sent: \"%s\"\n",clientstring);
	fprintf(stdout,"Connecting to socket, and sending message....\n");
	
	//using connect function we can use sock_id to write and read messages to sock_id
	if ( connect(sock_id,(struct sockaddr *)&ser, sizeof(ser)) !=0 )
	{
		perror("While calling connect()");
		exit(1);
	}
	
	//write to sock_id, client string of size strsize
	out = write( sock_id, (char*)clientstring, strsize );

	//check size for error
	if( out < 0 )
	{
		  perror("While writing to socket");
		  exit(1);   
	}
	
    //read data then print to and from sock_id fd
	in = read( sock_id, (char*)serverstring, strsize );
	
	//check size for error
	if( in < 0 )
	{
		  perror("While reading from socket");
		  exit(1);   	
	}
	
	fprintf(stdout, "Encrypted Message[Server Response]: \"%s\"\n", serverstring);
	close(sock_id); //close socket
	
}
