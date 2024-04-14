

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

void *thread_function(void *arg) {

    int thread_id = *((int *) arg); // Extract thread ID from argument
    printf("Thread %d: Starting\n", thread_id);

    for (int i = 0; i < 3; i++) {
        printf("Thread %d: Executing iteration %d\n", thread_id, i+1);
        sleep(1);
    }
    
    printf("Thread %d: Exiting\n", thread_id);
    pthread_exit(NULL);
}

int main() {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    struct sched_param param;
    param.sched_priority = 0;

    // Experiment with different scheduling policies
     //pthread_attr_setschedpolicy(&attr, SCHED_FIFO); // FIFO
    pthread_attr_setschedpolicy(&attr, SCHED_RR);   // Round Robin
    //pthread_attr_setschedpolicy(&attr, SCHED_OTHER); // Default

    pthread_t threads[4];
    int thread_ids[4] = {1, 2, 3, 4}; // Thread IDs

    // Create threads with specified scheduling policy
    for (int i = 0; i < 4; i++) {
        printf("Creating a thread\n");
        pthread_create(&threads[i], &attr, thread_function, (void *) &thread_ids[i]);
        //sleep(1);
    }

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