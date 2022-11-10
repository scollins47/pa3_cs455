/*********************************************************************
    PA-03:  Sockets

    FILE:   subMirror.c   SKELETON 

    Written By: 
		1- Griffin Gaskins
		 
    Submitted on:   November 2022
**********************************************************************/

#include    "myNetLib.h"

/*------------------------------------------------------------------------
 * This is a child server attending to an incoming client on 'sd'
 * Audit activities to the Auditor via 'sd_audit'
 *------------------------------------------------------------------------
 */


int main( int argc , char *argv[] )
{
    int sd, sd_audit ;
    
    char *developerName = "GRIFFIN GASKINS" ;
    
    printf( "\n****  sub-Mirror Server **** by %s\n\n" , developerName ) ;

        
    // Get the required  socket descriptors of the Client
    // and of the Auditor from the command line arguments

    sd        = atoi( argv[ 1 ]) ;  // client connected TCP socket
    sd_audit  = atoi( argv[ 2 ]) ;  // Auditor UDP socket
    
    // find out my IP:Port of the client from 
    // the provided socket descriptors
    struct sockaddr_in  cliaddr ;
    socklen_t            clilen ;
    cliaddr.sin_family = AF_INET ;
    clilen = sizeof( cliaddr ) ;
    audit_t      activity ;
    getpeername( sd , ( SA*) &cliaddr , &clilen) ;

    char *clientIP = inet_ntoa( cliaddr.sin_addr ) ;
    unsigned short clientPort = ntohs( cliaddr.sin_port ) ;

      
    while ( 1 )   // Loop until client closes socket
    {
        // Get a chunk of data from the client. Wisely choose which 
        // variant of the read() wrappers to use here
        int n ;
        char buf[ 512 ] ;
        
        if ((n = Read( sd, buf, sizeof( buf ))) == 0) 
        {
            printf( "Client closed connection\n" ) ;
            break ;
        }
        printf( "%d bytes received from client %s:%u\n" ,
            n , clientIP , clientPort ) ;

        // Send the received data to the Auditor
        // wisely choose which variant of the write() wrappers to use here
        if ( writen( sd_audit , buf , n ) != n)
        {
            printf( "Send error to Auditor\n" ) ;
            break ;
        }
       // Send all bytes received above back to the client
        if ( writen( sd , buf , n ) != n)
        {
            printf( "Send error to client\n" ) ;
            break ;
        }
        activity.op = 2 ;
        activity.nBytes = n ;
        activity.ip = inet_addr( AUDITOR_IP ) ;
        if ( sendto( sd_audit , &activity , sizeof(activity) , 0 , ( SA*) &cliaddr , clilen ) != sizeof(activity))
        {
            printf( "Send error to Auditor\n" ) ;
            break ;
        }
    }
    Close ( sd ) ;
    Close( sd_audit ) ;
    printf( "Sub-Mirror Server DONE\n" ) ;    

    return 0;
}
