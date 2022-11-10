/*********************************************************************
    PA-03:  Sockets

    FILE:   myNetLib.c   SKELETON

    Modified by ABOUTABL from original UNP's code 
	
	Code completed by:
		1- Write Student Name Here	
		
    Submitted on:   November 2022
**********************************************************************/

#include    "myNetLib.h"
#define     MAXSTRLEN 256

//------------------------------------------------------------
// To be called after a function that sets errno 

void err_sys( const char* x ) 
{ 
    fflush( stderr ) ;
    perror(x); 
    exit(1); 
}

//------------------------------------------------------------
// To be called after a function that DOES NOT set errno 

void err_quit( const char* x ) 
{ 
    fflush( stderr ) ;
    fputs( x , stderr ) ; 
    exit(1); 
}

//------------------------------------------------------------
/* Wrapper for fork() */

pid_t Fork(void) 
{
    pid_t pid;

    if ( (pid = fork() ) < 0)
	    err_sys("Fork() error");

    return pid;
}

//------------------------------------------------------------
/* Wrapper for sigaction */

Sigfunc * sigactionWrapper( int signo, Sigfunc *func )
{
	struct sigaction	act, oact;

	act.sa_handler = func;
	sigemptyset( &act.sa_mask );
	act.sa_flags   = 0;

	if( sigaction(signo, &act, &oact) < 0 )
		return( SIG_ERR );
	return( oact.sa_handler );
}

//------------------------------------------------------------
/* Wrapper for listen() */

int Listen(int sockfd, int backlog)
{
    int     errCode ;
    char    str[ MAXSTRLEN ] ;

    errCode = listen( sockfd , backlog ) ;

    if (  errCode < 0 )
    {
        snprintf( str , MAXSTRLEN , "\nFailed to listen() on socket %d" , 
                  sockfd );
        err_sys( str ) ;
    }
    return sockfd;
}

//------------------------------------------------------------
/*   A wrapper for the accept() slow system call. 
   If interrupted by a signal then retry, otherwise error   */

int Accept( int fd, struct sockaddr *sa, socklen_t *salenptr )
{
	int		n;
again:
    if ( (n = accept(fd, sa, salenptr)) < 0) {
#ifdef  EPROTO
    if (errno == EPROTO || errno == ECONNABORTED)
#else
    if (errno == ECONNABORTED)
#endif
        goto again;
    else
        err_sys("accept error");
    }
    return(n);
}

//------------------------------------------------------------
/* Wrapper for close() */

void Close( int fd )
{
	if ( close( fd ) == -1 )
		err_sys("Close() error");
}

//-------------------------------------------------------------
/* A wrapper for the read() slow system call. 
   If interrupted by a signal then retry, otherwise error
   It's OK to short count even if no EOF encountered         */

ssize_t Read( int fd, void *buff, size_t n )
{
    int nread ;
    
    if ( ( nread = read( fd , buff , n ) ) < 0 )
    {
        if ( errno == EINTR )
            return Read( fd , buff , n ) ;
        err_sys( "Read() error" ) ;
    }

    return nread ;
}

//-------------------------------------------------------------
/* A wrapper for the read() slow system call. 
   Keep reading until exactly "n" bytes are from a descriptor. 
   Short-counts only if EOF, otherwise error                 */

ssize_t readn( int fd, void *vptr, size_t n )
{
	size_t	nleft;
	ssize_t	nread;
	char	*ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nread = read(fd, ptr, nleft)) < 0) {
            if (errno == EINTR)
                nread = 0;      /* and call read() again */
            else
                return(-1);
        } else if (nread == 0)
            break;              /* EOF */

        nleft -= nread;
        ptr   += nread;
    }

    return(n - nleft);		/* return >= 0 */
}

/*-------------------------------------------------------------------*/
/* A wrapper for the above readn()*/
/* Insist on reading exactly n bytes from fd, otherwise fail */

ssize_t Readn( int fd, void *vptr, size_t n )
{
    ssize_t nread ;

    if ( ( nread = readn( fd , vptr , n ) ) != n )
        err_sys( "Readn() error" ) ;

    return nread ;

}

/*-------------------------------------------------------------------*/
/* A wrapper for the write() sometimes-slow system call. */
/* Insist on writing exactly nbytes to fd, otherwise fail */

ssize_t	writen( int fd, const void *vptr, size_t n )
{
	size_t		nleft;
	ssize_t		nwritten;
	const char	*ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0;       /* and call write() again */
            else
                return(-1);         /* error */
        }

        nleft -= nwritten;
        ptr   += nwritten;
    }
    return(n);


    return( n );
}

/*------------------------------------------------------------------------
 * socketTCP - allocate & connect a socket using TCP
 *------------------------------------------------------------------------
 * Arguments:
 *      s_port    - If not zero, the local source port to bind to. 
 *                  Usually set by a server, but a client can optionally do it, too.
 *      remoteIP  - IP (in dotted-decimal) of remote host
 *      d_port    - the destination port at remote host
 *                  To connect if-and-only-if remoteIP != NULL  &&  d_port != 0
 */

int socketTCP( uint16_t s_port , const char *remoteIP, uint16_t d_port ) 
{
	int	   sd ;	    /* socket descriptor */

    /* Allocate a TCP socket */
    if ((sd = socket( AF_INET, SOCK_STREAM, 0 )) < 0){
        perror( "socketTCP: socket failed" ) ;
        return (sd) ;
    }
    
    // If desired by caller, bind to the provided s_port
    // Usually a server does this, but a client may also do it
    if( s_port > 0 )
    {
        struct sockaddr_in  localaddr;  /* local address */

        memset( (char *)&localaddr, 0, sizeof(localaddr) );
        localaddr.sin_family      = AF_INET;
        localaddr.sin_port        = htons(s_port);
        localaddr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(sd, (struct sockaddr *) &localaddr, sizeof(localaddr)) < 0){
            close(sd); 
            return (sd);
        }

        printf("\nTCP socket %u is bound to local %s : %hu\n" , sd,  inet_ntoa( localaddr.sin_addr ) , ntohs(localaddr.sin_port));
    }

    // If desired, connect to remoteIP:port
    // Usually a client does this
    if( remoteIP != NULL  && d_port != 0 ){
        struct sockaddr_in  remoteaddr; /* remote address */

        /* Fill in the remote server's IP address and port */
        memset( (char *)&remoteaddr, 0, sizeof(remoteaddr) );
        remoteaddr.sin_family      = AF_INET;
        remoteaddr.sin_port        = htons(d_port);
        if (inet_aton( remoteIP, &remoteaddr.sin_addr ) == 0) {
            close(sd); 
            return (sd);
        }

        // Connect socket to remote host
        if (connect(sd, (struct sockaddr *) &remoteaddr, sizeof(remoteaddr)) < 0) {
            close(sd); 
            return (sd);
        }
        printf("\nTCP socket %u is connected to remote %s : %hu\n" , sd, inet_ntoa( remoteaddr.sin_addr ) , ntohs(remoteaddr.sin_port));
    }

	return sd;
}

/*------------------------------------------------------------------------
 * socketUDP - allocate & connect a socket using UDP
 *------------------------------------------------------------------------
 * Arguments:
 *      s_port    - If not zero, the local source port to bind to. 
 *                  Usually set by a server.
 *      remoteIP  - IP (in dotted-decimal) of remote host
 *      d_port    - the destination port at remote host
 *                  To connect if-and-only-if remoteIP != NULL  &&  d_port != 0
 */

int socketUDP( uint16_t s_port , const char *remoteIP, uint16_t d_port ) 
{
	int	   sd ;	    /* socket descriptor */


    /* Allocate a UDP socket */
    sd = socket( AF_INET, SOCK_DGRAM, 0 );
    if ( sd < 0 ) 
    {
        perror( "socketUDP: socket failed" ) ;
        return sd;
    }
    
    // If desired by caller, bind to the provided Source Port
    // Usually a server does this, but a client may also do it
    if( s_port > 0 ) {
        /* Bind to an available port */
        struct sockaddr_in localaddr ;  /* Our address */

        memset( &localaddr, 0, sizeof(localaddr) );  /* Zero out structure */
        localaddr.sin_family = AF_INET;
        localaddr.sin_addr.s_addr = htonl( INADDR_ANY );  /* Any incoming interface */
        localaddr.sin_port = htons(s_port);

        if ( bind( sd, (struct sockaddr *) &localaddr, sizeof(localaddr) ) < 0 ) 
        {
            perror( "socketUDP: bind failed" ) ;
            return sd;
        }
    }

    // If desired, restrict this socket to remoteIP:port
    // The caller may do this to use this UDP socket with ONLY one specific remote host
    if( remoteIP != NULL  && d_port != 0 )
    {
        /* Connect to the server */
        struct sockaddr_in remoteaddr ;  /* Remote address */

        memset( &remoteaddr, 0, sizeof(remoteaddr) );  /* Zero out structure */
        remoteaddr.sin_family = AF_INET;
        remoteaddr.sin_port = htons( d_port );
        remoteaddr.sin_addr.s_addr = inet_addr( remoteIP ); 
        
        if ( connect( sd, (struct sockaddr *) &remoteaddr, sizeof(SA) ) < 0 ) 
        {
            perror( "socketUDP: connect failed" ) ;
            return sd;
        }

        printf("UDP socket %u is restricted to remote %s : %hu\n" , sd , remoteIP , d_port ) ;
    }

	return sd;
}
