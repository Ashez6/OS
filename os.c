
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

cpu_set_t cpu_set;

// Function executed by each thread

void *thread_function(void *arg) {

    // Set thread affinity to a specific CPU core

    if(pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu_set)!=0){
        printf("affinity errorrrr\n");
    }
    
    int thread_id = *((int *) arg); // Extract thread ID from argument
    printf("Thread %d: Starting\n", thread_id);

    // Perform some work (iterations)

    printf("Thread %d: Executing iteration %d\n", thread_id, 1);
    for(int i=0;i<80000000;i++){

    }
    printf("Thread %d: Executing iteration %d\n", thread_id, 2);
    for(int i=0;i<80000000;i++){

    }
    printf("Thread %d: Executing iteration %d\n", thread_id, 3);
    for(int i=0;i<80000000;i++){

    }
    printf("Thread %d: Executing iteration %d\n", thread_id, 4);
    for(int i=0;i<80000000;i++){

    }
    
    printf("Thread %d: Exiting\n", thread_id);
    pthread_exit(NULL);
}


int main() {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    struct sched_param param;
    param.sched_priority = 99;
    

    pthread_t threads[4];
    int thread_ids[4] = {1, 2, 3, 4}; // Thread IDs
    
    //Create a cpu set for the threads to use
    CPU_ZERO(&cpu_set);
    CPU_SET(0, &cpu_set);

      // Set scheduling policy and priority for the main thread

    if(pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu_set)!=0){
        printf("Error setting main thread affinity\n");
    }
    
    if(pthread_attr_setinheritsched(&attr,PTHREAD_EXPLICIT_SCHED)!=0){
        printf("Error setting thread inheritance\n\n");
    }

// Set scheduling policy and priority for worker threads 
    if(pthread_setschedparam(pthread_self(),SCHED_RR,&param)!=0){
        printf("Error setting thread attribute policy\n");
    }

   
    if(pthread_attr_setschedpolicy(&attr, SCHED_RR)!=0){
        printf("Error setting thread attribute policy\n");
    }
    if(pthread_attr_setschedparam(&attr,&param)!=0){
        printf("Error setting thread attribute priority\n");
    }
    
       
       
  // Create and join threads

  for (int i = 0; i < 4; i++) {
        pthread_create(&threads[i], &attr, thread_function, (void *)&thread_ids[i]);
    }
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