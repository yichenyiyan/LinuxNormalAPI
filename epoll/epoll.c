#include<assert.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/epoll.h>

#define  PORT  9999
#define  QUEUELEN  10
#define  MAX_EVENTS 1024
#define  BUFSIZE   1024


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





int main()
{
    char buffer1[BUFSIZE] = {0};
    int sockfd, epfd, clientfd, eventcout;
    struct sockaddr_in clientaddr;
    socklen_t addrlen = sizeof(clientaddr);
    sockfd = socketInit();
    epfd = epoll_create1(0);
    if(epfd < 0){
        perror("epoll_create1");
        close(sockfd);
        exit(-1);
    }

    struct epoll_event ev, events[MAX_EVENTS];
    memset(events, 0, sizeof(events));
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev) < 0){
        perror("epoll_ctl sockfd");
        close(sockfd);
        exit(-2);
    }


    while(1){
        eventcout = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (eventcout == -1) {
            perror("epoll_wait");
            exit(-3);
        }

        for(int i = 0; i < eventcout; ++i){
            if(events[i].data.fd == sockfd){
                if((clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &addrlen)) < 0){
                    perror("accept");
                    continue;
                }
                // 将新连接的客户端套接字添加到 epoll 实例中
                ev.events = EPOLLIN;
                ev.data.fd = clientfd;
                if (epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev) == -1) {
                    perror("epoll_ctl");
                    exit(EXIT_FAILURE);
                }
                
                printf("New client connected\n");
            }
            else{
                int fd = events[i].data.fd;
                bzero(buffer1, sizeof(buffer1));
                ssize_t rbytes = recv(fd, buffer1, sizeof(buffer1), 0);
                if (rbytes == -1) {
                    perror("read");
                    exit(EXIT_FAILURE);
                } else if (rbytes == 0) {
                    // 客户端关闭连接
                    printf("Client disconnected\n");
                    close(fd);
                } else {
                    printf("Received: %s\n", buffer1);
                    send(fd, buffer1, strlen(buffer1), 0);
                }
            }
        }
    }
	close(sockfd);
    return 0;
}
