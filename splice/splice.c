#define _GNU_SOURCE
#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<unistd.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<fcntl.h>



int main( int argc, char** argv )
{
	if( argc <= 2 )
	{
		printf( " usage format error!\n " );
		return 1;
	}

	const char* ip = argv[1];
	int port = atoi( argv[2] );

	struct sockaddr_in address;
	bzero( &address, sizeof( address ) );
	address.sin_family = AF_INET;
	inet_pton( AF_INET, ip, &address.sin_addr );
	address.sin_port = htons( port );


	int sock = socket( PF_INET, SOCK_STREAM, 0 );
	assert( sock >= 0 );
	
	int ret = bind( sock, (struct sockaddr*)&address, sizeof( address ) );
	assert( ret != -1);

	struct sockaddr_in client;
	socklen_t addrlen = sizeof( client );
	int confd = accept( sock, ( struct sockaddr* )&client, &addrlen );

	if( confd < 0 )
	{
		printf( "errno is: %d\n",errno );
	}
	else
	{
		int pipefd[2];
		assert(ret != -1);
		ret =pipe( pipefd );
		//将confd上流入的客户端数据定向到管道中
		ret = splice( confd, NULL, pipefd[1], NULL, 32768,
				SPLICE_F_MORE | SPLICE_F_MOVE );
		assert( ret != -1 );
		//将管道的输出定向到confd客户端链接文件描述符
		ret = splice( pipefd[0], NULL, confd, NULL, 32768,
				SPLICE_F_MORE | SPLICE_F_MOVE );
		assert( ret != -1 );
		close( confd );
	}

	close( sock );

	return 0;
}
