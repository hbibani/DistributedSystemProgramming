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

#define BUF_LEN 512
/* sighandler to check if socket has closed if buffer exceeded*/
void handler(int handle)
{	
	fprintf( stderr , "Socket closed...exiting....\n" );
}

int main( int argc , char* argv[ ] )
{
	int sock_id;
	/*sock address will contain the protocol, port and address via the other inaddr struct*/
	struct sockaddr_in ser;
	
	/*used by gethostbyname which returns a struct hostent*/
	struct hostent *host;
	int serlen;
	int strsize;
	short serport;

	/*the return values of rcvfrom or sendto*/
	int out, in; 
	char clientstring[ BUF_LEN ];
	char serverstring[ BUF_LEN ];
	
	/* 
	   sigaction handler to deal with buffer exceeded when socket
	   is closed on the other side 
	*/
	struct sigaction new_actn;
	new_actn.sa_handler = handler;
	sigemptyset(&new_actn.sa_mask);
	new_actn.sa_flags = 0;
	sigaction(SIGPIPE, &new_actn, NULL);
	
	if( argc != 3 )
	{
		fprintf( stderr , "Usage: %s ser Port send_string\n" , argv[ 0 ] );
		exit( EXIT_FAILURE );
	}
	
	/*The gethostbyname() function returns a structure of type hostent the given host name.*/
	host = gethostbyname( argv [ 1 ] ); 
	
	if( host == NULL )
	{
		herror( "While calling gethostbyname()" );
		exit( EXIT_FAILURE );
	}
	
	serport = atoi( argv[ 2 ] ); 		//get the port
	
	
	/*
		we retrieve a socket_id similar to that of an fd
		specify the domain, type, protocol [usually 0]
		the sock_DGRAM gives the type of  protocol used connectionless, unreliable [udp]
	*/
	sock_id = socket( PF_INET , SOCK_STREAM, 0 );
	
	if( sock_id < 0 )
	{
		perror( "While calling socket()" );
		exit( EXIT_FAILURE );
	}
	
	ser.sin_family = AF_INET; //get the server protocol for struct sockaddr, ipv4 protocol
	

	/*
		hostent was retruned by gethostbyname that took ip address gets information
		hostent has all the neccessary information for the address
		memcpy copies the information from hostent and places it into the sock structure
	*/
	memcpy( &ser.sin_addr.s_addr , host->h_addr_list[ 0 ] , host->h_length );
	
	/*htons converts port into network readable address*/
	ser.sin_port = htons( serport );
	/*to detemrine the size of read and write calls*/
	strsize = strlen( clientstring ) + 1; 
	fprintf( stdout , "Connecting to socket, and sending message....\n" );
	
	/*using connect function we can use sock_id to write and read messages to sock_id*/
	if ( connect( sock_id , ( struct sockaddr * )&ser , sizeof( ser ) ) !=0 )
	{
		perror( "While calling connect()" );
		exit( 1 );
	}
    
	/* 	
		set up string for initial read, the server will send an initial string, 
		and the initial & will be an indicator to read the first time the while
		loop initiates; this is more simpler than have an initial read before 
		the while loop
	*/
	clientstring[ 0 ] = '&';
	clientstring[ 1 ] = '\0';
	while( 1 )
	{	
		/* initiate string , read data then print to and from sock_id fd */
		serverstring[ 0 ] = '\0';
		if( clientstring[ strlen( clientstring ) - 1 ] == '&' )
		{
			in = read( sock_id , ( char* )serverstring , 512 );
	
			/*check value of read, if less than zero then error, if 0 then the server shut down*/
			if( in < 0 )
			{
				perror( "While reading from socket" );
				break;	
			}
			
			/*print and get input string from user*/
			printf( "%s" , serverstring );
		}
		
		/* Get client string */
		scanf( "%s" , clientstring );
		/* check for exit character */
		if( clientstring[ 0 ] == 'X' )
		{
			break;
		}
		/* write to sock_id, client string of size strsize */
		out = write( sock_id , ( char* )clientstring , strlen( clientstring ) + 1 );

		/*check size for error*/
		if( out < 0 )
		{
			perror( "While writing to socket" );
			break;   
		}
	}
	
	/*close socket*/
	close( sock_id ); 
	fprintf( stdout , "Exiting program....\n" );
}
