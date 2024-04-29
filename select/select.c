#include<sys/select.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<stdio.h>
#include<unistd.h>
#include<assert.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include <sys/types.h>

//select 与单个客户端进行通信

//#define IP "0.0.0.0"
#define PORT 8888
#define QUEUELEN 10
#define BUFFERSIZE 1024


void 
setnonblocking(int fd)
{
    fcntl(fd, F_SETFD, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}

int 
socketInit()
{
    int opt = 1;
    struct sockaddr_in srev;
    memset(&srev, 0, sizeof(srev));
    srev.sin_family = AF_INET;
    srev.sin_addr.s_addr = INADDR_ANY;
    srev.sin_port = htons(PORT);
    socklen_t addrlen = sizeof(srev);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    
    
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        printf("getsockopt\n");
        assert(0);
    }
    if(bind(fd, (struct sockaddr*)&srev, addrlen) < 0){
        printf("bind\n");
        assert(0);
    }
    if(listen(fd, QUEUELEN) < 0){
        printf("listen\n");
        assert(0);
    }
    
    return fd;
}


int main(int argc, char** argv)
{
#if 0
    if(argc != 3){
        printf("usage format:%s ip port!\n", argv[0]);
        exit(-1);
    }
#endif

    int max_fd, conn_fd;
    struct sockaddr_in client;
    char buffer1[BUFFERSIZE] = {0}; //send
    char buffer2[BUFFERSIZE] = {0}; //recv


    int fd = socketInit();

    setnonblocking(fd);

    fd_set read_mask;
    fd_set all_reads;

    FD_ZERO(&all_reads);
    FD_SET(0, &all_reads);
    FD_SET(fd, &all_reads);

    memset(&client, 0, sizeof(client));
    socklen_t clientlen = sizeof(client);
    max_fd = fd;

    while(1){
        read_mask = all_reads;
        int n = select(max_fd + 1, &read_mask, NULL, NULL, NULL);
        if(n < 0){
            perror("select error!");
            close(fd);
            exit(1);
        }

        if(FD_ISSET(fd, &read_mask)){
            memset(&client, 0, sizeof(client));
            conn_fd = accept(fd, (struct sockaddr*)&client, &clientlen);
            if(conn_fd < 0){
                perror("accept error!");
                continue;
            }

            if(conn_fd > max_fd){
                max_fd = conn_fd;
            }
            FD_SET(conn_fd, &all_reads);
            printf("new client connected his fd is: %d.\n", conn_fd);
        }

        if(FD_ISSET(STDIN_FILENO, &read_mask)){
            memset(buffer1, 0, sizeof(buffer1));
            if(conn_fd <= 0){
                fgets(buffer1, sizeof(buffer1), stdin);
                printf("please wait for a connection.\n");
                continue;
            }

            if(fgets(buffer1, sizeof(buffer1), stdin) != NULL){
                int len = strlen(buffer1);
                if(buffer1[len - 1] == '\n')
                    buffer1[len - 1] = '\0';
                printf("send to client: %s\n", buffer1);

                ssize_t send_bytes = send(conn_fd, buffer1, strlen(buffer1), 0);
                if(send_bytes < 0)
                    perror("fail to send!");
                else
                    printf("send bytes: %ld\n", send_bytes);
            }
        }

        if(FD_ISSET(conn_fd, &read_mask)){
            memset(buffer2, 0, sizeof(buffer2));
            ssize_t recv_bytes = recv(conn_fd, buffer2, sizeof(buffer2), 0);
            if(recv_bytes < 0){
                printf("recv error!\n");
                continue;
            }
            if(recv_bytes == 0){
                printf("client disconnected who fd is %d\n", conn_fd);
                FD_CLR(conn_fd, &all_reads);
                close(conn_fd);
                continue;
            }

            printf("from client message: %s\n", buffer2);

            send(conn_fd, buffer2, strlen(buffer2), 0);
        }

    }

    close(fd);

    return 0;
}
