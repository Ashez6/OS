

#include <pthread.h>
#include <stdio.h> 
#include <stdlib.h>

void* func1(void* arg)
{ 
printf("The first thread contains \n");
pthread_t id = pthread_self();
int sum=2+3;
printf ("id no1 %lu\n", (unsigned long) id);
printf ("The sum of the 2 numbers is %d \n", sum);
pthread_exit(NULL); 
} 

void* func2(void* arg) 
{ 
printf("The second thread contains\n"); 
pthread_t id = pthread_self();
int sub=2-3;
printf ("id no2 %lu\n", (unsigned long) id);
printf ("The difference  of the 2 numbers is %d \n", sub);
pthread_exit(NULL); 
} 

void* func3(void* arg) 
{ 
printf("The third thread contains\n"); 
pthread_t id = pthread_self();
int mul=2*3;
printf ("id no3 %lu\n", (unsigned long) id);
printf ("The multiplication of the 2 numbers is %d \n", mul);
pthread_exit(NULL); 
} 



void* func4(void* arg) 
{ 
printf("The fourth thread contains\n"); 
pthread_t id = pthread_self();
int div =2/3;
printf ("id no4 %lu\n", (unsigned long) id);
printf ("The division of the 2 numbers is %d \n", div);
pthread_exit(NULL); 
} 

// int main() {
   
//     struct sched_param params;
//     int policy=SCHED_RR;
//     params.sched_priority = 50; // Priority ranges from 1 (lowest) to 99 (highest)
//     pthread_t id1;
//     pthread_t id2;
//     pthread_t id3;
//     pthread_t id4;
//     pthread_attr_t * attr;
//     attr = (pthread_attr_t *)malloc(sizeof(pthread_attr_t));

//     pthread_attr_setschedpolicy(attr,policy);
//     pthread_attr_setschedparam(attr, &params);

//     pthread_getschedparam(attr, &policy, &params);
//     printf("Current scheduling policy: %s, priority: %d\n", 
//            (policy == SCHED_FIFO)  ? "SCHED_FIFO" :
//            (policy == SCHED_RR)    ? "SCHED_RR" :
//            (policy == SCHED_OTHER) ? "SCHED_OTHER" : "UNKNOWN",
//            params.sched_priority);

    
//     pthread_setschedparam(pthread_self(),policy,&params);
//     // Creating new threads 
//     pthread_create(&id1, attr, &func1, NULL); 

//     params.sched_priority = 40;
//     pthread_attr_setschedparam(attr, &params);
//     pthread_create(&id2, attr, &func2, NULL); 
    
//     params.sched_priority = 80;
//     pthread_attr_setschedparam(attr, &params);
//     pthread_create(&id3, attr, &func3, NULL); 
   
//     params.sched_priority = 60;
//     pthread_attr_setschedparam(attr, &params);
//     pthread_create(&id4, attr, &func4, NULL);

    
//     printf("This line may be printed before thread terminates\n"); 
//     // Compare the two threads created 
//     if(pthread_equal(id1, pthread_self())) 
//     printf("Thread 1  is equal to main\n");  
//     else
//     printf("Thread 1 is not equal to main\n"); 
//     if(pthread_equal(id2, pthread_self())) 
//     printf("Thread 2  is equal to main\n"); 
//     else
//     printf("Thread 2 is not equal to main\n"); 
//     if(pthread_equal(id3, pthread_self())) 
//     printf("Thread 3 is equal to main\n");  
//     else
//     printf("Thread 3 is not equal to main\n"); 
//     if(pthread_equal(id4, pthread_self())) 
//     printf("Thread 4 is equal to main\n");  
//     else
//     printf("Thread 4 is not equal to main\n");  

//     // Waiting for the created thread to terminate 
//     pthread_join(id1, NULL);
//     pthread_join(id2, NULL);
//     pthread_join(id3, NULL);
//     pthread_join(id4, NULL);
//     printf("All threads are joined \n"); 
//     pthread_exit(NULL); 
//     return 0; 
// }

void *thread_function(void *arg) {
    int thread_id = *((int *) arg); // Extract thread ID from argument
    printf("Thread %d: Starting\n", thread_id);

    for (int i = 0; i < 3; i++) {
        printf("Thread %d: Executing iteration %d\n", thread_id, i+1);
        sleep(1); // Simulate some work
    }
    
    printf("Thread %d: Exiting\n", thread_id);
    pthread_exit(NULL);
}

int main() {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    struct sched_param param;
    param.sched_priority = 0;
    int policy;
    struct sched_param param2;
    int error;
    
    error=pthread_setschedparam(pthread_self(),SCHED_FIFO,&param);
    if(error) printf("first error");

    // Experiment with different scheduling policies
     error=pthread_attr_setschedpolicy(&attr, SCHED_FIFO); // FIFO
     if(error) printf("second error");
     //pthread_attr_setschedpolicy(&attr, SCHED_RR);   // Round Robin
    //pthread_attr_setschedpolicy(&attr, SCHED_OTHER); // Default (Usually Round Robin)

    pthread_t threads[4];
    int thread_ids[4] = {1, 2, 3, 4}; // Thread IDs

    // Create threads with specified scheduling policy
    for (int i = 0; i < 4; i++) {
        printf("Creating a thread\n");
        error=pthread_create(&threads[i], &attr, thread_function, (void *) &thread_ids[i]);
        if(error) printf("third error");
    }

    // Join threads
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}