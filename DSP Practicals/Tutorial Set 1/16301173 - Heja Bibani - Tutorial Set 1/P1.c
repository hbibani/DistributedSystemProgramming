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
	
	//the port
	short serport;
	int out, in; //the return values of rcvfrom or sendto
	
	//strings to send out and in
	char clientstring[BUF_LEN];
	char serverstring[BUF_LEN];
	
	if(argc !=4)
	{
		fprintf(stderr, "Usage: %s ser Port send_string\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	//The gethostbyname() function returns a structure of type hostent using the given host name.
	host = gethostbyname(argv[1]); 
	
	if(host == NULL)
	{
		herror("While calling gethostbyname()");
		exit(EXIT_FAILURE);
	}
	
	serport = atoi(argv[2]); 		//get the port from command line
	strcpy(clientstring, argv[3]);  //get the string from command line
	
	/*
		we retrieve a socket_id similar to that of an fd
		specify the domain [Internet domain], type, protocol [usually 0]
		the sock_DGRAM gives the type of  protocol used connectionless, unreliable [udp]
	*/
	sock_id=socket(PF_INET, SOCK_DGRAM, 0);
	
	if(sock_id<0)
	{
		perror("While calling socket()");
		exit(EXIT_FAILURE);
	}
	
	ser.sin_family = AF_INET; //get the server protocol for struct sockaddr = ipv4 protocol
	
	/*
		hostent was returned by gethostbyname that took ip address gets information
		hostent has all the neccessary information for the address
		memcpy copies the information from hostent and places it into the sock structure
	*/
	memcpy(&ser.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
	
	//htons converts port into network readable address
	ser.sin_port = htons(serport);
	strsize = strlen(clientstring) + 1;
	
	
	/*
		socket fd was created by socket() system call, and then placed at the front
		We send the string [including the size] to the location specified  by sock addr
		ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
	*/
	out = sendto(sock_id, clientstring, strsize, 0, (struct sockaddr*)&ser,sizeof(ser));
	
	//check size of return for error
	if(out<0)
	{
		perror("While calling sendto()");
		exit(EXIT_FAILURE);
	}
	
	fprintf(stdout,"Sent string: \"%s\"\n",clientstring);
	fprintf(stdout,"About to recieve from the other side\n");
	serlen=sizeof(ser);
	
	/*
		socket fd was created by socket() system call, and then placed at the front
		The string is recieved from destination of the server specified in sockaddr
	*/
	in = recvfrom(sock_id,serverstring, BUF_LEN, 0, (struct sockaddr*)&ser, (socklen_t*)&serlen);
	
	
	//check size of return for error
	if(in<0)
	{
		perror("While calling recvfrom()");
		exit(EXIT_FAILURE);
	}
	
	fprintf(stdout, "Server's response[Encrypted Message]: \"%s\"\n", serverstring);
	close(sock_id); //close socket
		
}
