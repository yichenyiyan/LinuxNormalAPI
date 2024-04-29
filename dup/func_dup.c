#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>


/*dup函数创建一个新的文件描述符，该新文件描述符和原有文件描述符file_descriptor指向相同的文件、管道或者网络连接。并且 dup返回的文件描述符总是取系统当前可用的最小整数值。dup2和 dup类似，不过它将返回第一个不小于file_descriptor_two的整数值。dup和dup2系统调用失败时返回-1并设置errno.
*/


int main(int argc, char** argv)
{
	if(argc <= 2){
		printf("usage:%s ip_address port_number\n", argv[0]);
		return 1;
	}
	
	const char* ip = argv[1];
	int port = atoi(argv[2]);

	struct sockaddr_in address;
	bzero(&address, sizeof(address));

	address.sin_family = AF_INET;
	inet_pton( AF_INET, ip, &address.sin_addr );
	address.sin_port = htons( port );

	int sock = socket( PF_INET, SOCK_STREAM, 0);
	assert( sock >= 0);
#if 0
	ret = listen(sock, 5);
	assert(ret != -1);
#endif
	int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));
	assert(ret != -1);

	ret = listen(sock, 5);
        assert(ret != -1);


	struct sockaddr_in client;
	socklen_t client_addresslength = sizeof( client );
	int confd = accept( sock, (struct sockaddr*)&client, &client_addresslength );
	if( confd < 0 )
	{
		printf("error is: %d\n",errno );
	}
	else
	{
		close(STDOUT_FILENO);
		dup( confd );
	       printf( "abcd\n" );
	       close(confd);
	}

	close( sock );
	


	return 0;
}
