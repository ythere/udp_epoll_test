#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <assert.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <epollfd.h>

#define PORT 8888
#define BUF_SIZE 1024

#define SERVADDR "192.168.2.206"


int main( int argc, char *argv[] )
{
	struct sockaddr_in addr, serveraddr;
	char buf[BUF_SIZE];
	int strlen;
	int recvlen = 0; 
	
	bzero( &addr, sizeof( addr ) );
	addr.sin_family = AF_INET;
	addr.sin_port = htons( PORT );
	addr.sin_addr.s_addr = htonl( INADDR_ANY );
	
	int sockfd = socket( AF_INET, SOCK_DGRAM, 0 );
	
	if ( bind ( sockfd, ( struct sockaddr* )&addr, sizeof( addr ) ) == -1 )
	{
		printf( "bind error\n" );
	}
	
	bzero( &serveraddr, sizeof( serveraddr ) );
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons( PORT );
	serveraddr.sin_addr.s_addr = inet_addr( SERVADDR );
	
	socklen_t len;
	for ( int i = 0; i < 3; i++ )
	{
		memset( buf, 0, BUF_SIZE );
		fputs( "Please input your data\n", stdout );
		fgets( buf, BUF_SIZE, stdin );
		
		if ( !strcmp( buf, "q\n" ) || !strcmp( buf, "Q\n" ) )
			break;
		strlen = sendto ( sockfd, buf, BUF_SIZE, 0, ( struct sockaddr* )&serveraddr, sizeof( serveraddr ) );
		recvlen = recvfrom( sockfd, buf, BUF_SIZE, 0, ( struct sockaddr* )&serveraddr, &len );
		printf( "服务器回传消息： %s\n", buf );
		}
	}
	return 0;
}
