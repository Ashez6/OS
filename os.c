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
printf ("The multiplication of the 2 numbers is %d \n", div);
pthread_exit(NULL); 
} 

int main() {

pthread_t id1;
pthread_t id2;
pthread_t id3;
pthread_t id4;
// Creating new threads 
pthread_create(&id1, NULL, &func1, NULL); 
pthread_create(&id2, NULL, &func2, NULL); 
pthread_create(&id3, NULL, &func3, NULL); 
pthread_create(&id4, NULL, &func4, NULL); 

printf("This line may be printed before thread terminates\n"); 
// Compare the two threads created 
if(pthread_equal(id1, pthread_self())) 
printf("Thread 1  is equal to main\n");  
else
printf("Thread 1 is not equal to main\n"); 
if(pthread_equal(id2, pthread_self())) 
printf("Thread 2  is equal to main\n"); 
else
printf("Thread 2 is not equal to main\n"); 
if(pthread_equal(id3, pthread_self())) 
printf("Thread 3 is equal to main\n");  
else
printf("Thread 3 is not equal to main\n"); 
if(pthread_equal(id4, pthread_self())) 
printf("Thread 4 is equal to main\n");  
else
printf("Thread 4 is not equal to main\n");  

// Waiting for the created thread to terminate 
pthread_join(id1, NULL);
pthread_join(id2, NULL);
pthread_join(id3, NULL);
pthread_join(id4, NULL);
printf("All threads are joined \n"); 
pthread_exit(NULL); 
return 0; 
}