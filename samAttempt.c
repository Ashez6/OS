#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>


#define NUM_THREADS 4

void *thread_func(void *t) {
    long tid = (long)t;
      printf("Thread %ld: Starting\n", thread_id);

    for (int i = 0; i < 3; i++) {
        printf("Thread %ld: Executing iteration %d\n", thread_id, i+1);
    }
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    pthread_attr_t attr;
    struct sched_param param;
    cpu_set_t cpuset;

    pthread_attr_init(&attr);

    // Set the scheduling policy to round-robin
    pthread_attr_setschedpolicy(&attr, SCHED_RR);

    param.sched_priority = sched_get_priority_max(SCHED_RR);
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
