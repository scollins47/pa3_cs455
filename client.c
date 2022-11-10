/*********************************************************************
    PA-03:  Sockets

    FILE:   client.c   SKELETON

    Written By: 
		1- Write Student Name Here	
		 
    Submitted on:   November 2022
**********************************************************************/

#include    "myNetLib.h"

void mirrorFile( int in , int copy , int mirror , int audit );

int main( int argc , char *argv[] )
{
    int     sd_mirror ,     // Socket to Mirror TCP server
            sd_audit  ;     // Socket to Auditor UDP server
    int     queLen = 10 ;   // Max #of pending connection requests

    char *mirrorIP  = MIRROR_IP ,                   // default Mirror Server
         *auditorIP = AUDITOR_IP ,                  // Default Auditor Server
         *inFile    = "GoldilockAnd3Bears.txt" ;    // Default input file

    printf( "\nClient by ABOUTABL has Started\n" );

    // Get the command-line arguments

    switch ( argc )
    {
        case 4: auditorIP = argv[3] ;
        case 3: mirrorIP  = argv[2] ;
        case 2: inFile    = argv[1] ;
        case 1: break ;

        default:
            printf("\nInvalid argument(s). Usage: %s <inputFileName> [mirror-IP]"
            " [auditor-IP]\n\n" , argv[0] ) ;
            exit(-1);        
    }

    printf("Working with these arguments:\n" ) ;
    printf("\tInput   File Name is '%s'\n" , inFile    ) ;
    printf("\tMirror  Server IP is '%s'\n" , mirrorIP  ) ;
    printf("\tAuditor Server IP is '%s'\n" , auditorIP ) ;

    int  fd_in , fd_cpy ;

    // Open the input file and create the copy file by same name.copy

    if ( ( fd_in = open( inFile , O_RDONLY ) ) < 0 ) 
        err_sys("Can't open input file");

    char copyFile[80] ;
    sprintf( copyFile , "%s.copy" , inFile );

    if ( ( fd_cpy = open( copyFile , O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR ) ) < 0 ) 
        err_sys("Can't create copy file");

    // Use sockettCP() to create a local TCP socket with ephemeral port, and connect it to
    // the mirror server at  mirrorIP : MIRROR_TCP_PORT
    if ( ( sd_mirror = socketTCP( MIRROR_TCP_PORT , mirrorIP , MIRROR_TCP_PORT ) ) < 0 )
        err_sys("Can't create a socket to Mirror server");
    printf("TCP Client is now connected to the TCP Mirror server %s : %hu\n" , mirrorIP , MIRROR_TCP_PORT ) ;

    { 
        // This block to be implemented in Phase Two
    
        // Use socketUDP to created an ephemeral local UDP socket and restrict 
        // its peer to the Auditor server
        if ( ( sd_audit = socketUDP( AUDITOR_UDP_PORT , auditorIP , AUDITOR_UDP_PORT ) ) < 0 ){
            err_sys("Can't create a socket to Auditor server");
        }
        printf("UDP Client is now connected to the UDP Auditor server %s : %hu\n" , auditorIP , AUDITOR_UDP_PORT );
    
    }

    // Now, Start moving data: fd_in ==> sd_mirror ==> fd_cpy
    // While logging all send and receive transactions to
    // the Auditor UDP Server
    mirrorFile( fd_in , sd_mirror , fd_cpy  , sd_audit  ) ;
    
    puts("TCP Client finished sending the local file to the TCP Mirror server");
    // Close( sd_mirror ) ;  // Observe the traffic when we use close() vs shutdown()
    shutdown( sd_mirror , SHUT_WR ) ;
    puts("\nTCP Client closed the connection to the TCP Mirror server\n");
    
    return 0;
    
}

/*------------------------------------------------------------------------
 * Trasfer data from descriptor 'in' to descriptor 'mirror' 
 * and receive it back through descriptor 'mirror'. 
 * // This is for Phase Two: Report sending & receiving transactions to descriptor 'audit'
 *------------------------------------------------------------------------*/
 
#define CHUNK_SZ  1000
#define MAXSTRLEN 256

void mirrorFile( int in , int mirror , int copy , int audit )
{
    unsigned char buf[ CHUNK_SZ ] , buf2[ CHUNK_SZ ]  , str[MAXSTRLEN];
    audit_t  activity ; // This is for Phase Two
    struct sockaddr_in      mySocket, mirrorServer ;
    int    alen ;
    
    // Learn my IP:Port associated with 'mirror' 
    alen = sizeof( mySocket ) ;
    getsockname( mirror , (SA *) &mySocket , &alen ) ;
    printf( "TCP Client IP = %s , Port = %hu\n" , inet_ntoa( mySocket.sin_addr ) , ntohs( mySocket.sin_port ) ) ;

    // Learn the IP:Port of my peer on the other side of 'mirror'     
    alen = sizeof( mirrorServer ) ;
    getpeername( mirror , (SA *) &mirrorServer , &alen ) ;
    printf( "Mirror TCP Server IP = %s , Port = %hu\n" , inet_ntoa( mirrorServer.sin_addr ) , ntohs( mirrorServer.sin_port ) ) ;

    // Repeat untill all data has been sent and received back
    // As this happens, save the received copy to the 'copy' file descriptor
    while ( 1 )
    {
        // Get up to CHUNK_SZ bytes from input file  and send ALL of what I get
        // to the 'mirror' socket
        int n;
        n = Read( in , buf , CHUNK_SZ );
        if ( n == 0 ){
            break ; // EOF
        } else if ( n < 0 ){
            err_sys("Read error") ;
        }
        printf("Read %d bytes from the input file.\n" , n );
        writen(mirror, buf, n);

        // This block to be implemented in Phase Two
        // by setting the fields of 'activity'        
        // Report this sending activity to the Auditor
        activity.op = sent;
        activity.nBytes = n;
        activity.ip = mirrorServer.sin_addr.s_addr;
        writen(audit, &activity, sizeof(activity));
       
        // Now read from 'mirror' EXACTLY the same number of bytes I sent earlier
        int bytes_read = readn(mirror, buf2, n);
        if (bytes_read != bytes_read){
            err_sys("Read error");
        }

        // This block to be implemented in Phase Two
        // Report this receiving activity to the Auditor
        // by setting the fields of 'activity'
        activity.op = received;
        activity.nBytes = bytes_read;
        activity.ip = mirrorServer.sin_addr.s_addr;
        writen(audit, &activity, sizeof(activity));
                
        // Finally, save a copy of what I received back to the 'copy' file. 
        writen(copy, buf2, bytes_read);
    }
    
}
