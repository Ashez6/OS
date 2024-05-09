#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdbool.h>
#include <ctype.h>

// Define constants
#define MEMORY_SIZE 60
#define MAX_PROCESSES 3
#define MAX_QUEUES 4

// Quantum times for different levels
#define QUANTUM_LVL_1 1
#define QUANTUM_LVL_2 2
#define QUANTUM_LVL_3 4
#define QUANTUM_LVL_4 8

typedef enum
{
    NEW,
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
} ProcessState;

typedef struct
{
    int process_id;
    ProcessState state;
    int priority;
    int program_counter;
    int memory_start;
    int memory_end;
} PCB;

char memory[60][2];
int used=0;


typedef struct {
    int head;
    int tail;
    int size;
    PCB *queue[MAX_PROCESSES];
} Queue;

typedef struct {
    enum {zero,one} value;
    Queue queue;
    int ownerID;
}Mutex;

void semWait(Mutex *m , PCB *p) {
    if (m->value == one) {
        m->ownerID = p->process_id;
        m->value = zero;
    } else {
        (&(m->queue))->queue[(&(m->queue))->size]=p;
        (&(m->queue))->size++;
        (&(m->queue))->tail++;
    }
}


void semSignal(Mutex *m, PCB *p) {
    if(m->ownerID == p->process_id) {
        if ((&(m->queue))->size==0)
            m->value = one;
        else {
        /* remove a process P from m.queue and place it on ready list*/
        /* update ownerID to be equal to Process P’s ID */
        }
    }
}




void initialize_mutex(Mutex *mutex)
{
    mutex->value=one;
    mutex->queue->head = 0;
    mutex->queue->tail = 0;
    mutex->queue->size = 0;
}

PCB *create_process(int id, int priority, char* functionName,int memory_start, int memory_end)
{
    PCB *pcb = (PCB *)malloc(sizeof(PCB));
    pcb->memory_start = used;
    memory[used++]=*pcb;
    pcb->process_id = id;
    pcb->state = NEW;
    pcb->priority = priority;
    int start = read_program_to_memory(functionName);
    pcb->program_counter = start;
    pcb->memory_end = used+2;

    return pcb;
}


int read_program_to_memory(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Error opening file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    int start = memory[used];
    char buffer[50];

    while (fgets(buffer, sizeof(buffer), file))
    {
        if (used >= MEMORY_SIZE)
        {
            fprintf(stderr, "Memory overflow.\n");
            exit(EXIT_FAILURE);
        }
        strcpy(memory[used], buffer);
        used++;
    }

    fclose(file);
    return start; // Returns the start index in memory
}



int main(){
    used = 0;
    memset(memory, 0, sizeof(memory));
    return 0;
}