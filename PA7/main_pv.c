#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int number = 10;     // 全局变量，表示临界区资源
int reader_count = 0; // 读者计数
sem_t write_block;  // 用于写者的信号量，管理是否允许写操作
sem_t mutex;        // 用于保护reader_count的信号量

// 为了方便和课本/课件上的P/V信号量操作伪代码对应，增加可读性
// 进行了封装换名，其实是不必要的
void P(sem_t* lock){sem_wait(lock); }
void V(sem_t* lock){sem_post(lock); }

// 读者线程函数
void* pth_read(void* arg) {
    long id = (long)arg;

    P(&mutex);
    reader_count++;
    if (reader_count == 1) {  // 第一个读者特殊处理
        P(&write_block);
    }
    V(&mutex);

    // 读操作
    printf("Reader %ld: %d\n", id, number);
    sleep(1);

    P(&mutex);
    reader_count--;
    if (reader_count == 0) {  // 最后一个读者特殊处理
        V(&write_block);
    }
    V(&mutex);

    pthread_exit(0);
}

// 写者线程函数
void* pth_write(void* arg) {
    long id = (long)arg;

    P(&write_block); // 获取写锁
    // 写操作
    number++;
    printf("Writer %ld: %d\n", id, number);
    sleep(1);
    V(&write_block); // 释放写锁

    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    int num_read_threads =  atoi(argv[2]);
    int num_write_threads = atoi(argv[4]);
    pthread_t readers[num_read_threads], writers[num_write_threads];

    // 初始化信号量
    sem_init(&write_block, 0, 1);
    sem_init(&mutex, 0, 1);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start); // 记录开始时间

    //创建读者线程
    for (long i = 0; i < num_read_threads; i++) {
        pthread_create(&readers[i], NULL, pth_read, (void*)i);
    }
    // 创建写者线程
    for (long i = 0; i < num_write_threads; i++) {
        pthread_create(&writers[i], NULL, pth_write, (void*)i);
    }

    // 等待所有线程完成
    for (int i = 0; i < num_read_threads; i++) {
        pthread_join(readers[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end); // 记录结束时间
    double elapsed_time1 = (end.tv_sec - start.tv_sec);

    for (int i = 0; i < num_write_threads; i++) {
        pthread_join(writers[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end); // 记录结束时间
    double elapsed_time2 = (end.tv_sec - start.tv_sec);

    printf("read time: %f seconds\n", elapsed_time1);
    printf("write time: %f seconds\n", elapsed_time2-elapsed_time1);

    // 销毁信号量
    sem_destroy(&write_block);
    sem_destroy(&mutex);

    return 0;
}

// int main()
// {
//     pthread_t threads[10];

//     // 初始化信号量
//     sem_init(&write_block, 0, 1);
//     sem_init(&mutex, 0, 1);
//     for (long i = 0; i < 10; i++) 
//     {
//         if(i%2==0)
//         pthread_create(&threads[i], NULL, pth_read, (void*)i);
//         else
//         pthread_create(&threads[i], NULL, pth_write, (void*)i);
//     }

//     for (int i = 0; i < 10; i++) {
//         pthread_join(threads[i], NULL);
//     }

//         // 销毁信号量
//     sem_destroy(&write_block);
//     sem_destroy(&mutex);

//     return 0;

// }
