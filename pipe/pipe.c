#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//管道简单使用案例

int main(int argc, char** argv) 
{
    int fd[2]; // 文件描述符数组，fd[0]用于读取，fd[1]用于写入
    pid_t pid;
    char message[] = "Hello, Pipe!";

    // 创建管道
    if (pipe(fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // 创建子进程
    pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) { // 父进程
        close(fd[0]); // 关闭读取端

        // 向管道写入数据
        write(fd[1], message, sizeof(message));
        close(fd[1]); // 关闭写入端
    }
    else { // 子进程
        close(fd[1]); // 关闭写入端

        char buffer[100];

        // 从管道读取数据
        read(fd[0], buffer, sizeof(buffer));
        printf("Child process received: %s\n", buffer);
        close(fd[0]); // 关闭读取端
    }

    return 0;
}
