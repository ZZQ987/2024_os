#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define BUFFER_SIZE 20

int main() {
    int pipefd[2];
    pid_t pid1, pid2;

    // 创建管道
    pipe(pipefd);

    pid1 = fork();

    if (pid1 == 0) {
        // 子进程1: 执行cat

        // 关闭读端
        close(pipefd[0]);

        // 第一种

        // 将标准输出重定向
        // 实际就是将cat要输出的内容截取放到管道里
        dup2(pipefd[1], STDOUT_FILENO);

        // 关闭管道写端
        close(pipefd[1]);

        // 使用 execvp 执行 "cat"
        char *cat_args[] = {"cat", "test1.txt", "test2.txt", NULL};
        execvp("cat", cat_args);
        
        // 第二种

        // int fd1 = open("test1.txt", O_RDONLY);
        // int fd2 = open("test2.txt", O_RDONLY);

        // // 读取文件内容并写入管道
        // char buffer[BUFFER_SIZE];
        // ssize_t bytesRead;
        
        // while ((bytesRead = read(fd1, buffer, BUFFER_SIZE)) > 0) {
        //     write(pipefd[1], buffer, bytesRead);
        // }

        // while ((bytesRead = read(fd2, buffer, BUFFER_SIZE)) > 0) {
        //     write(pipefd[1], buffer, bytesRead);
        // }
        

        // close(fd1);
        // close(fd2);
        // close(pipefd[1]);

        return 0;
    } 

    pid2 = fork();

    if (pid2 == 0) {
        // 子进程2: 执行sort

        // 关闭写端
        close(pipefd[1]);

        // 将标准输入重定向
        // 接受刚才写入数据
        dup2(pipefd[0], STDIN_FILENO);

        // 关闭读端
        close(pipefd[0]);

        // 使用 execvp 执行 "sort"
        char *sort_args[] = {"sort", NULL};
        execvp("sort", sort_args);

        // 因为子进程2的标准输出没有被重定向
        // 所以sort 的结果就输出到命令行中了
    }

    // 父进程: 关闭管道两端并等待子进程结束
    close(pipefd[0]);
    close(pipefd[1]);

    // 注意这里是进程等待结束
    // 不是前几次PA写的线程等待结束 
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    return 0;
}
