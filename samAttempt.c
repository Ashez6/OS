#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>

#define NUM_THREADS 4
#define NUM_ITERATIONS 3

int policy = SCHED_FIFO; 

int counter = 0;

void *thread_func(void *t) {
    long tid = (long)t;
    if(policy == SCHED_RR){
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        while (counter % NUM_THREADS != tid); // Busy wait
        printf("Thread %ld: Executing iteration %d\n", tid, i+1);
        counter++;
    }
    }

    if (policy == SCHED_FIFO || policy == SCHED_OTHER){
        for (int i = 0; i < NUM_ITERATIONS; i++)
        printf("Thread %ld: Executing iteration %d\n", tid, i+1);
    }
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    pthread_attr_t attr;
    struct sched_param param;
    cpu_set_t cpuset;

    pthread_attr_init(&attr);

    pthread_attr_setschedpolicy(&attr, policy);

    param.sched_priority = sched_get_priority_max(policy);
    pthread_attr_setschedparam(&attr, &param);

    // Initialize the CPU set
    CPU_ZERO(&cpuset);

    for (long t = 0; t < NUM_THREADS; t++) {
        // Add each CPU to the set
        CPU_SET(t, &cpuset);

        pthread_create(&threads[t], &attr, thread_func, (void *)t);

        // Set the CPU affinity for the thread
        if (pthread_setaffinity_np(threads[t], sizeof(cpu_set_t), &cpuset) != 0) {
            perror("pthread_setaffinity_np");
            exit(EXIT_FAILURE);
        }

        // Clear the CPU set for the next iteration
        CPU_ZERO(&cpuset);
    }

    for (int t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }

    pthread_attr_destroy(&attr);

    return 0;
}
