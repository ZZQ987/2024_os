#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int number=10;//作为临界区变量
pthread_rwlock_t rwlock;

void *pth_read(void* arg){
    long pid = (long)arg;
    pthread_rwlock_rdlock(&rwlock);//加读锁
    printf("read_thread%ld: %d\n",pid,number);
    sleep(1);
    pthread_rwlock_unlock(&rwlock);
    pthread_exit(0);
}


void *pth_write(void* arg){
    long pid = (long)arg;
    pthread_rwlock_wrlock(&rwlock);//加写锁
    number++;                      //对临界区变量进行修改(写操作)
    printf("write_thread%ld: %d\n",pid,number);
    sleep(1);
    pthread_rwlock_unlock(&rwlock);
    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    int num_read_threads =  atoi(argv[2]);
    int num_write_threads = atoi(argv[4]);
    pthread_t readers[num_read_threads], writers[num_write_threads];
    pthread_rwlock_init(&rwlock, NULL);

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
    pthread_rwlock_destroy(&rwlock);

    clock_gettime(CLOCK_MONOTONIC, &end); // 记录结束时间
    double elapsed_time2 = (end.tv_sec - start.tv_sec);

    printf("read time: %f seconds\n", elapsed_time1);
    printf("write time: %f seconds\n", elapsed_time2-elapsed_time1);

    return 0;
}

// int main()
// {
//     pthread_t threads[10];

//     pthread_rwlock_init(&rwlock, NULL);
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

//     pthread_rwlock_destroy(&rwlock);

//     return 0;

// }