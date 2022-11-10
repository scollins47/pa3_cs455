/*********************************************************************
    PA-03:  Sockets

    FILE:   mirror-exec.c   SKELETON

    Written By: 
		1- Write Student Name Here	
		 
    Submitted on:   November 2022
**********************************************************************/

#include    "myNetLib.h"
#include <stdlib.h>

void reaper(int sig) ;      // Handle SIGCHLD
void killHandler(int sig) ; // Handle SIGTERM

//------------------------------------------------------------
int main( int argc , char *argv[] )
{
    int     sd_listen ,             // Receiving Socket descriptor to Server
            sd_audit ;              // Socket descriptor with Auditor
    int     queLen = 10 ;           // Max #of pending connection requests
    struct sockaddr_in  cl_addr;    // the address of a client

    char *developerName = "Sammy Collins Griffin Gaskins" ;
    
    printf( "\n****  Mirror Server **** by %s\n\n" , developerName ) ;


    char *auditorIP = AUDITOR_IP ;      // Default Auditor Server

    // Get the optional Auditon IP address from argv, otherwise use default
    switch ( argc )
    {
        case 2:
            auditorIP = argv[1] ;
            break;
        case 1:
            break;
        default:
            err_quit("Usage: Mirror [AuditorIP]");
            break;

    }

    printf("Working with these arguments:\n" ) ;
    printf("\tAuditor Server IP is '%s'\n" , auditorIP ) ;


    // Create a TCP socket bound to this local port MIRROR_TCP_PORT
    sd_listen = socketTCP( MIRROR_TCP_PORT, NULL, 0 ) ;
    if ( sd_listen == -1 ) {
        err_quit("socketTCP error");
    }

    // Create a UDP socket with ephemeral port, but 'connected' to the Auditor server
    sd_audit = socketUDP( AUDITOR_UDP_PORT , auditorIP , AUDITOR_UDP_PORT ) ;
    if  ( sd_audit == -1 ){
        err_quit("socketUDP error");
    }

    // Now, start listening on this TCP port
    listen( sd_listen, queLen ) ;
    printf( "Mirror Server Started. Listening at socket %hu\n" , sd_listen );

    /* Let reaper clean up after completed child processes */
    sigactionWrapper( SIGCHLD, reaper ) ;
    
    /* Let killHandler() handle CTRL-C or a KILL command from terminal*/
    sigactionWrapper( SIGTERM, killHandler ) ;

   
    // For ever, wait till clients connect to me
    // Will only terminate when I receive a 'SIGTERM'

    while(1)
    {
        // Wait for a client to connect & record the 'accepted' socket in sd_clnt
        socklen_t peer_addr_size;
        peer_addr_size = sizeof(struct sockaddr_in);

        int sd_clnt = Accept( sd_listen, (struct sockaddr*)&cl_addr, &peer_addr_size);
        if ( sd_clnt == -1 ) {
            err_quit("accept error") ;
        } 

        // Display IP : Port of the client
        printf( "Client %s : %d connected\n", inet_ntoa(cl_addr.sin_addr), ntohs(cl_addr.sin_port));

        // Delegate a sub-server child process to handle this client
        // Start a subMirror server using one of the 'exec' family of system calls
        // Pass the 'sd_clnt'  and  'sd_audit' to that subServer
         if( Fork() == 0 ) {
            // This is a sub-server child process
            // Close the socket listening for clients
            Close(sd_listen);
            // Execute the sub-server program which is the same program as this one
            // But the server must pass the 'sd_clnt' and 'sd_audit' as args
	    char str_sdClnt[10];
	    char str_sdAudit[10];
	    snprintf(str_sdClnt, 10, "%d", sd_clnt);
	    snprintf(str_sdAudit, 10, "%d", sd_audit);
            int err = execlp("./subMirror" , "./subMirror", str_sdClnt, str_sdAudit, (char *) NULL);
            if (err == -1)
            	err_quit("execlp error\n");
        }
        
        // Close the socket descriptor for this client
        Close( sd_clnt ) ;

    }

    // As for the parent server, make sure you close sockets you don't need
    // Close the server's 'listening' socket
    Close( sd_listen ) ;
    
    // Close the socket descriptor for the Auditor server
    Close( sd_audit ) ;   

    return 0;
}

/*------------------------------------------------------------------------
 * reaper - clean up zombie children
 *------------------------------------------------------------------------
 */
 
void reaper(int sig)
{
    pid_t  pid ;
	int	   status;

	// Don't know how many signals, so loop till there are still 
    // more signals from child processes

    while ( ( pid = waitpid( -1, &status, WNOHANG ) ) > 0 ) {
        // Check if child process terminated normally or not
        // if not, print a message, else, print nothing
        fprintf( stderr , "Child server process %d has terminated\n" , pid );
    }
    return ;
}

/*------------------------------------------------------------------------
 * killHandler - clean up after receiving a KILL signal
 *------------------------------------------------------------------------
 */
void killHandler(int sig)
{
    fprintf( stderr , "\nThe Mirror Server is now closing\n" );
    exit(0) ;
}
