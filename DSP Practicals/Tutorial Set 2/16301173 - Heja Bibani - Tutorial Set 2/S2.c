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

/* port number for the server to use, see note above for setting it */
#define SERVER_TCP_PORT 9999


/* These are used in manage_connection(), the numbers aren't critical or
   magic they can be set to anything, but BUF_LEN must be greater than
   COM_BUF_LEN
*/
#define BUF_LEN 512
#define COM_BUF_LEN 32

/* child manages connections and executes this code */
void manage_connection( int , int );
/*child recieves string and alters it and sends it back to client*/
int server_processing( char *in_string , char *out_string );
/*kill children with signal handler function*/
void handle_sigcld( int );


int main()
{
    int rsock_id;       /* socket descriptor for rendezvous socket */ 
    int esock_id;       /* socket descriptor for each connected client */
    int err_code; 		/* Error Code holder */
    int clength;                         
    int pid;        	/* pid of created child */
    struct sockaddr_in server,client;
    struct hostent *cltdet;                         
    struct sigaction child_sig;       
                                         

    fprintf( stderr , "M: The Server is starting up...\n" );
    
	 /* Set up sigaction for children handling */
    child_sig.sa_handler = handle_sigcld;
    sigfillset( &child_sig.sa_mask );
    child_sig.sa_flags   = SA_RESTART | SA_NOCLDSTOP;
    sigaction( SIGCHLD , &child_sig , NULL );


    /* create stream socket TCP */
    rsock_id = socket( PF_INET , SOCK_STREAM , 0 );
    if( rsock_id < 0 )
    {
        perror( "M: rendezvous socket" );
        exit( EXIT_FAILURE );
    }
    
    fprintf( stderr , "M:\tsocket_id=%d\n" , rsock_id );
    
    
    /* setup address for socket structure then bind */
    memset( &server , 0 , sizeof( server ) );
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl( INADDR_ANY );
    server.sin_port = htons( SERVER_TCP_PORT );
    
    /* bind the socket */
    if( ( err_code = bind( rsock_id , ( struct sockaddr * ) &server , sizeof( server ) ) ) < 0 )
    {
        perror( "M: Binding" );
        exit( EXIT_FAILURE );
    }
    
    fprintf( stderr , "M:\tbound\n" );
    /* limit to 5 listen activate connection */
    if( ( err_code = listen( rsock_id , 5 ) ) < 0 )
    {
        perror( "M: Listen error" );
        exit( EXIT_FAILURE );
    }
    
    fprintf( stdout , "M:\tlistening\n" );
    fprintf( stdout , "M: ...ready to accept connections\n" );
    
    while( 1 )
    {
        /* server loop created, setup to accet sockets */
        clength = sizeof( client );
        if( ( esock_id = accept( rsock_id , ( struct sockaddr * ) &client ,
                                        ( socklen_t * ) &clength ) ) < 0 )
        {
            perror( "M: connetion error" );
            exit( EXIT_FAILURE );
        }
        
		 /* get the name of the client so we can print it */
        cltdet = gethostbyaddr( ( void* )&client.sin_addr.s_addr , 4 , AF_INET );
        
        if( cltdet == NULL )
        {
            herror( "M: resolving the clients address error" );
            exit( EXIT_FAILURE );
        }
        fprintf( stderr , "M: Accepted a connection from %s on port %d with esock_id=%d\n" ,
            cltdet->h_name , ntohs( client.sin_port ) , esock_id );
                        
        /* child handles connection */
        if( ( pid = fork( ) ) == 0 )
        {
            /* close socket then manage the connection, then exit */
            close( rsock_id );
            manage_connection( esock_id , esock_id );
            exit( EXIT_SUCCESS );
        }
        else
        {
            /* Parent specifies which child handles connection */
            close( esock_id );
            fprintf( stderr , "M: process %d will service this.\n" , pid );
        }
    }
    /*close the socket */
    close( rsock_id );
}



/*child recieves string and alters it and sends it back to client*/
int server_processing( char *in_string , char *out_string )
{
    int i, length;
    char c;

    length = strlen( in_string );

    for( i = 0 ; i < length ; i++ )
    {
        c = tolower( in_string[ i ] );
        if( c == in_string[ i ] )
                out_string[ i ] = toupper( in_string[ i ] );
        else
                out_string[ i ] = c;
    }
    out_string[ length ] = '\0';
    
    return length;
}

/* child manages connections and executes this code */
void manage_connection( int in , int out )
{       
	/* read count and buffer count */
    int read_count,buffer_count, recv_count;
	/* string buffers for various requirments */   
    char input_buffer[ BUF_LEN ] , output_buffer[ 1024 ] , recv_buffer[ BUF_LEN ] , hostname[ 40 ] , inbuf[ BUF_LEN ];
    /* bit at start of output messages to show which child */                
    char pfix[ 100 ]; 
    int i;
	char end_of_data = '&';
    /* get name of server, then print to send out */
    gethostname( hostname , 40 );
    /* get server strings prepared, and then print some initial statements to client */
    sprintf( pfix , "\tC %d: " , getpid( ) );
    fprintf( stdout ,"\n%sStarting up\n" , pfix );
    sprintf( output_buffer ,"\n\nExample Server on host: %s, enter 'X' to exit\n" , hostname );
    write( out 	, output_buffer , strlen( output_buffer ) + 1 );
    /*zero strings for preparation*/
    inbuf[ 0 ] = '\0';
    output_buffer[ 0 ] = '\0';
    
    
    while( 1 )
    {
	    /* 
	        while loop which continues communication with process, continously reading information
	        processing that information, sending the information back to client and then requesting 
			more information
	    */
	    buffer_count = 0;
	    while( 1 )
	    {	
	    	/* clear string read into buffer from outside */
			input_buffer[ 0 ] = '\0';
	        read_count = read( in , input_buffer , BUF_LEN );
	        if( read_count > 0 )
	        {
	        	/* check buffer size */
	            if( ( buffer_count + read_count ) > BUF_LEN )
	            {
	                fprintf( stderr ,"\n%s Buffer size exceeded!\n" , pfix );
	                close( in );
					exit( EXIT_FAILURE );
	            }
	        	
	        	/*null terminate end of input from read*/
	        	input_buffer[read_count] = '\0';
	            /* concatenate string and send back */
	            strcat( inbuf , input_buffer );
	            /* readjust and calculate buffer size */
                buffer_count = buffer_count + read_count;
	            /* print recieved data */
	            fprintf( stdout , "%sRead in [string :: size :: %ld]: %s\n" , pfix, strlen( input_buffer ) , input_buffer );
	            
	            if( input_buffer[ read_count - 2 ] == '&' ) 
					break;
	        }
	        else if( read_count == 0 )
	        {
	        	/* check read count for exit status */
	            fprintf( stderr , "\n%sClient has closed...closing\n" , pfix );
	            close( in );
	            exit( EXIT_FAILURE );
	        }
	        else
	        {
	        	/* if error exit */
	            sprintf( pfix , "\tC %d: Reading from connection" , getpid( ) );
	            perror( pfix );
	            close( in );
	            exit( EXIT_FAILURE );
	        }
	    }
		
		/* end connection if X is pressed */
	    if( inbuf[ 0 ] == 'X' ) 
		   break;
				
	    /*  get rid of null termination character and then process data */
	    inbuf[ strlen( inbuf ) - 1 ] = '\0';
	    recv_count = server_processing( inbuf , recv_buffer );
	
	    /* send it back with a message and next prompt */
	    sprintf( output_buffer ,"The server received %ld characters, rotated string:\n%s\n\nEnter next string: " , strlen( recv_buffer ) , recv_buffer );
	    write( out , output_buffer , strlen( output_buffer ) + 1 );
	    /*clear buffers for safety*/
	    inbuf[ 0 ] 			= '\0';
        output_buffer[ 0 ]  = '\0';
	    recv_buffer[ 0 ]    = '\0';
            
	}

	fprintf( stderr , "\n%sClient exited, closing down\n" , pfix );
	/*close socket*/
	close( in ); 
}

void handle_sigcld( int signo )
{
        pid_t child;
		/*wait for children and destroy all that is waiting*/
        while( 0 > waitpid( -1 , NULL , WNOHANG ) );
}
