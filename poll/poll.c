#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/poll.h>

//仅作了解，poll也用得比较少,还得是epoll

#define MAX_EVENTS 10
#define PORT 8080

int main(int argc, char** argv)
{
    int server_fd;
    int client_socket; 
    int num_ready;
    struct sockaddr_in address;
    struct pollfd fds[MAX_EVENTS];
    char buffer[1024] = {0};
    
    // 创建监听套接字
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // 设置地址重用
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // 绑定套接字
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    
    // 监听
    if (listen(server_fd, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    // 初始化 pollfd 结构体
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;
    
    printf("Server listening on port %d...\n", PORT);
    
    while (1) {
        // 调用 poll
        num_ready = poll(fds, MAX_EVENTS, -1);
        if (num_ready == -1) {
            perror("poll");
            exit(EXIT_FAILURE);
        }
        
        // 处理就绪的事件
        for (int i = 0; i < MAX_EVENTS; i++) {
            if (fds[i].revents & POLLIN) {
                if (fds[i].fd == server_fd) {
                    // 有新的连接请求
                    if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                        perror("accept");
                        exit(EXIT_FAILURE);
                    }
                    
                    printf("New client connected\n");
                    
                    // 将新连接的客户端套接字添加到 pollfd 数组中
                    for (int j = 1; j < MAX_EVENTS; j++) {
                        if (fds[j].fd == -1) {
                            fds[j].fd = client_socket;
                            fds[j].events = POLLIN;
                            break;
                        }
                    }
                } 
	 	else {
                    // 有数据可读
                    int bytes_read = read(fds[i].fd, buffer, sizeof(buffer));
                    if (bytes_read == -1) {
                        perror("read");
                        exit(EXIT_FAILURE);
                    } 
		    else if (bytes_read == 0) {
                        // 客户端关闭连接
                        printf("Client disconnected\n");
                        close(fds[i].fd);
                        fds[i].fd = -1;
                    } 
		    else {
                        // 处理接收到的数据
                        printf("Received: %s\n", buffer);
                        // 这里可以添加处理逻辑
                    }
                }
            }
        }
    }

    close(server_fd);    

    return 0;
}
