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
#include <semaphore.h>
#include <fcntl.h>
#include <sys/epoll.h>

//#include  "test_server.h"
#define PORT 8888
#define BACKLOG 10
#define MAX_SIZE 256
#define RECV_NUM 15
#define BUF_SIZE 1024

int counter = 0;
sem_t sem;

typedef struct Recvdata{
	int num;
	char countain[1024];
	struct Recvdata *next;
} Getdata;

Getdata *head = NULL;
Getdata *pEnd = NULL;

Getdata *create( int counter, char *buf )
{
	Getdata *pS;
	pS = malloc( sizeof( Getdata ) );
	pS->num = counter;
	strcpy( pS->countain, buf );
	if( head == NULL ){
		head = pS;
	} else {
		pEnd->next = pS;
	}
	pEnd = pS;
	return head;
}

void clearList( Getdata *x )
{
	Getdata *temp;
	if ( x == NULL )
		return -1;
	while ( x->next != NULL )
	{
		temp = x->next;
		free( temp );
		x = temp;
	}
	x->next = NULL;
	return 0;
}

int setnonblocking( int fd )
{
	int sta = fcntl( fd, F_GETFD, 0 ) | O_NONBLOCK;
	if ( fcntl( fd, F_SETFL, sta ) == -1 ){
		return -1;
	}
	return 0;
}

void processdata()
{
	while( 1 ){

		sem_wait( &sem );
		Getdata *process = head;
		int i;
		for (  i = 0; i < RECV_NUM; i++ ){
			printf( "%d\t%s\n", process->num, process->countain );	
			process = process->next;
		}
		clearList( head );
	}
}

int main( int argc, char *argv[] )
{
	struct sockaddr_in addr;
	struct sockaddr_in client_addr;
	
	int create_socket, epfd, nfds, i, reuse = 0;
	char buf[BUF_SIZE];
	int strlen;
	socklen_t len;
	
	struct epoll_event ev, events[20];
	
	bzero( &addr, sizeof( addr ) );
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons( PORT );
	
	create_socket = socket( AF_INET, SOCK_DGRAM, 0 );
	if ( create_socket > 0 )
		printf( "Create socket successful.\n" );
	else
		exit( 1 );
	
	if ( bind( create_socket, ( struct sockaddr* )&addr, sizeof( addr ) ) == 0 )
		printf( "Bind socket successful.\n" );
	else{
		perror( "bind wrong.\n" );
		exit( 1 );
	}
	
	int opt = 1;
	setsockopt( create_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof( opt ) );
	setsockopt( create_socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof( opt ) );
	if ( setnonblocking( create_socket ) < 0 ){
		perror( "setnonblocking error." ); 
	}
	
	sem_init( &sem, 0, 0 );
	
	pthread_t pt_1;
	int ret;
	ret = pthread_create( &pt_1, NULL, ( void * )processdata, NULL );
	assert( ret == 0 );
	
	epfd = epoll_create( 256 );
	ev.data.fd = create_socket;
	ev.events = EPOLLIN|EPOLLET;
	epoll_ctl( epfd, EPOLL_CTL_ADD, create_socket, &ev );
	
	while( 1 ) {
		nfds = epoll_wait( epfd, events, 20, -1 );
		if ( nfds == -1 )
		{
			perror( "epoll_wait" );
			break;
		}
		for ( i = 0; i < nfds; i++ )		{
			/*if ( events[i].data.fd == create_socket ){
				
				fputs( "EPOLLIN...socket", stdout );
				strlen = recvfrom( create_socket, buf, BUF_SIZE, 0, ( struct sockaddr* )&client_addr, &len );
				puts(buf);
				
				head = create( counter, buf );
			
				if ( counter == 14 ){
					sem_post( &sem );
					counter = 0;
				} else{
					counter++;
				}
				
				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = create_socket;
				epoll_ctl( epfd, EPOLL_CTL_ADD, create_socket, &ev );
			}*/
			 if ( events[i].events & EPOLLIN )
			{
				fputs( "EPOLLIN...", stdout );
				strlen = recvfrom( create_socket, buf, BUF_SIZE, 0, ( struct sockaddr* )&client_addr, &len );
				puts(buf);
				
				head = create( counter, buf );
			
				if ( counter == 14 ){
					sem_post( &sem );
					counter = 0;
				} else{
					counter++;
				}
				ev.data.fd = create_socket;
				ev.events = EPOLLOUT| EPOLLET;
				epoll_ctl( epfd, EPOLL_CTL_ADD, create_socket, &ev);
			}
		//sendto ( create_socket, "aaaaaaaAAA", BUF_SIZE, 0, ( struct sockaddr* )&client_addr, sizeof( client_addr) );
			}
		}

	return 0;
}
