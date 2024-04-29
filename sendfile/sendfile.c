#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/sendfile.h>



/*没什么好说的，使用sendfile发送文件更高效（零拷贝）
 * */


int main( int argc, char** argv )
{
	if( argc <= 3 )
	{
		printf("usage format error!\n");
		return 1;
	}

	const char* ip = argv[1];
	int port = atoi( argv[2] );
	const char* file_name = argv[3];

	int filefd = open( file_name, O_RDONLY );
	assert( filefd > 0 );
	struct stat stat_buf;
	fstat( filefd, &stat_buf );

	struct sockaddr_in address;
	bzero( &address, sizeof(address) );
	address.sin_family = AF_INET;
	inet_pton( AF_INET, ip, &address.sin_addr );
	address.sin_port = htons( port );

	int sock = socket( PF_INET, SOCK_STREAM, 0 );
	assert( sock >= 0 );

	int ret = bind( sock, ( struct sockaddr* )&address, sizeof( address ) );
	assert( ret != -1 );

	ret = listen( sock, 5 );
	assert( ret != -1 );

	struct sockaddr_in client;
	socklen_t addrlen = sizeof( client );
	int confd = accept( sock, ( struct sockaddr* )&client, &addrlen );
	if( confd < 0 )
	{
		printf( "error is %d\n",errno );
	}
	else
	{
		sendfile( confd, filefd, NULL, stat_buf.st_size );
		close( confd );
	}
	
	close( sock );


	return 0;
}
