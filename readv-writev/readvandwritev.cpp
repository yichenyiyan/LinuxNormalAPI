#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include<assert.h>
#include <sys/uio.h>

/*web服务器上的集中写
 * */

#define BUFFER_SIZE 1024



static const char* status_line[2] = { "200 OK", "500 Internal server error"};



int main( int argc, char** argv )
{
	
	if( argc <= 3 )
	{
		printf("usage: %s ip port filename\n", argv[0] );
		return 1;
	}

	const char* ip = argv[1];
	int port = atoi(argv[2]);
	


	const char* file_name = argv[3];
	struct sockaddr_in address;
	bzero( &address, sizeof( address ));
	address.sin_family = AF_INET;
	inet_pton( AF_INET, ip,&address.sin_addr );
	address.sin_port = htons( port );

	int sock = socket( PF_INET,SOCK_STREAM, 0 );
	assert(sock >= 0);
	int ret = bind( sock, (struct sockaddr*)&address, sizeof( address ) );
	assert( ret != -1 );

	ret = listen( sock, 5 );
	assert( ret != -1 );

	struct sockaddr_in client;
	socklen_t addrlen = sizeof( client );
	int confd = accept( sock,( struct sockaddr* )&client, &addrlen );
	if( confd < 0 )
	{
		printf( "errno is: %d\n",errno );
	}
	else
	{
		//用于保存HTTP应答的状态行、头部字段和一个空行的缓存区
		char header_buf[ BUFFER_SIZE ];
		memset( header_buf, '\0', BUFFER_SIZE );
		
		//用于存放目标文件内容的应用程序缓存
		char* file_buf;
		//用于获取目标文件的属性，比如是否为目录，文件大小等
		struct stat file_stat;

		//记录目标文件是否是有效文件
		bool valid = true;
		//缓存区header_buf目前已经使用了多少字节的空间
		int len = 0;
		if( stat( file_name,&file_stat ) < 0 )
		{
			valid = false;
		}
		else
		{
			if( S_ISDIR( file_stat.st_mode ) )
			{
				valid = false;
			}
			else if( file_stat.st_mode & S_IROTH ) //当前用户有读取目标文件的权限
			{
				int fd = open( file_name, O_RDONLY );
				file_buf = new char( file_stat.st_size + 1 );
				memset( file_buf,'\0',file_stat.st_size + 1 );
				if( read( fd,file_buf, file_stat.st_size ) < 0 )
				{
					valid = false;
				}
			}
			else
			{
				valid = false;
			}

		}
		if( valid )
		{
			ret = snprintf( header_buf, BUFFER_SIZE-1, "%s %s\r\n",
					"HTTP/1.1",status_line[0] );
			len += ret;
			ret = snprintf( header_buf + len, BUFFER_SIZE-1-len,
					"Content-Length: %ld\r\n", file_stat.st_size );
			len += ret;
			ret = snprintf( header_buf + len, BUFFER_SIZE-1-len, "%s", "\r\n");
			//利用writev将header_buf和file_buf的内容一并写出
			struct iovec iv[2];
			iv[ 0 ].iov_base = header_buf;
			iv[ 0 ].iov_len = strlen( header_buf );
			iv[ 1 ].iov_base = file_buf;
                        iv[ 1 ].iov_len = file_stat.st_size;
			ret = writev( confd, iv, 2 );
		}
		else
		{
			ret = snprintf(header_buf, BUFFER_SIZE-1, "%s %s\r\n",
					"HTTP/1.1", status_line[1] );
			len += ret; 
			ret = snprintf( header_buf + len, BUFFER_SIZE-1-len, "%s", "\r\n" );
			send( confd, header_buf, strlen( header_buf ), 0 );
		}
		close( confd );
		delete [] file_buf;
	}
	

	close(sock);
	
	
	return 0;

}
