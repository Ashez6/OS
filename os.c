
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

cpu_set_t cpu_set;

void *thread_function(void *arg) {

    if(pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu_set)!=0){
        printf("affinity errorrrr\n");
    }
    
    int thread_id = *((int *) arg); // Extract thread ID from argument
    printf("Thread %d: Starting\n", thread_id);

    for (int i = 0; i < 3; i++) {
        printf("Thread %d: Executing iteration %d\n", thread_id, i+1);
        //sleep(1);
    }
    
    printf("Thread %d: Exiting\n", thread_id);
    pthread_exit(NULL);
}

int main() {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    struct sched_param param;
    param.sched_priority = 1;

    pthread_t threads[4];
    int thread_ids[4] = {1, 2, 3, 4}; // Thread IDs
    
    //Create a cpu set for the threads to use
    CPU_ZERO(&cpu_set);
    CPU_SET(0, &cpu_set);

    if(pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu_set)!=0){
        printf("affinity error\n");
    }
    
    if(pthread_attr_setinheritsched(&attr,PTHREAD_EXPLICIT_SCHED)!=0){
        printf("inherit error\n");
    }

    if(pthread_setschedparam(pthread_self(),SCHED_RR,&param)!=0){
        printf("main sched error\n");
    }

   
    if(pthread_attr_setschedpolicy(&attr, SCHED_RR)!=0){
        printf("attr policy error\n");
    }
    if(pthread_attr_setschedparam(&attr,&param)!=0){
        printf("attr param error\n");
    }
    


    // Create threads with specified scheduling policy
    // for (int i = 0; i < 4; i++) {
    //     printf("Creating a thread\n");
    //     pthread_create(&threads[i], &attr, thread_function, (void *) &thread_ids[i]);
        
    //     //sleep(1);
    // }

    printf("Creating a thread\n");
    pthread_create(&threads[0], &attr, thread_function, (void *) &thread_ids[0]);
    printf("Creating a thread\n");
    pthread_create(&threads[1], &attr, thread_function, (void *) &thread_ids[1]);
    printf("Creating a thread\n");
    pthread_create(&threads[2], &attr, thread_function, (void *) &thread_ids[2]);
    printf("Creating a thread\n");
    pthread_create(&threads[3], &attr, thread_function, (void *) &thread_ids[3]);

    // Join threads
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

        // Measure CPU Utilization
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    printf("CPU Utilization: User Time = %ld.%06ld s, System Time = %ld.%06ld s\n",
           usage.ru_utime.tv_sec, usage.ru_utime.tv_usec,
           usage.ru_stime.tv_sec, usage.ru_stime.tv_usec);


    return 0;
}