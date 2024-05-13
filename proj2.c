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

typedef enum
{
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

typedef struct
{
    int head;
    int tail;
    int size;
    PCB *queue[MAX_PROCESSES];
} Queue;

typedef struct
{
    enum
    {
        zero,
        one
    } value;
    Queue queue;
    int ownerID;
} Mutex;

typedef struct
{
    char *name;
    void *data;
} Element;

Element *memory;
int used = 0;
Mutex inputMutex;
Mutex outputMutex;
Mutex fileMutex;
Queue generalBlockedQueue;
Queue readyQueues[MAX_QUEUES];
int cycle;
void printMemoryPCB(PCB *pcb);

void print_PCB(PCB *pcb)
{
    if (pcb == NULL)
    {
        fprintf(stderr, "Error: Null PCB pointer.\n");
        return;
    }

    printf("PCB : ");
    printf("Process ID: %d, ", pcb->process_id);
    switch (pcb->state)
    {
    case READY:
        printf("State: READY, ");
        break;
    case RUNNING:
        printf("State: RUNNING, ");
        break;
    case BLOCKED:
        printf("State: BLOCKED, ");
        break;
    case TERMINATED:
        printf("State: TERMINATED, ");
        break;
    default:
        printf("State: UNKNOWN, ");
        break;
    }
    printf("Priority: %d, ", pcb->priority);
    printf("Program Counter: %d, ", pcb->program_counter);
    printf("Memory Start: %d, ", pcb->memory_start);
    printf("Memory End: %d\n", pcb->memory_end);
}

void enqueue(Queue *queue, PCB *pcb)
{
    if (queue->size >= MAX_PROCESSES)
    {
        fprintf(stderr, "Error: Queue is full.\n");
        return;
    }
    queue->queue[queue->tail] = pcb;
    queue->tail = (queue->tail + 1) % MAX_PROCESSES;
    queue->size++;
}

PCB *dequeue(Queue *queue)
{
    if (queue == NULL)
    {
        fprintf(stderr, "Error: Null queue pointer during dequeue.\n");
        return NULL;
    }
    if (queue->size == 0)
    {
        fprintf(stderr, "Error: Queue is empty.\n");
        return NULL;
    }
    PCB *pcb = queue->queue[queue->head];
    queue->head = (queue->head + 1) % MAX_PROCESSES;
    queue->size--;
    return pcb;
}

void print_queue(Queue *queue)
{
    if (queue == NULL)
    {
        fprintf(stderr, "Error: Null queue pointer.\n");
        return;
    }

    for (int i = 0; i < queue->size; i++)
    {
        PCB *p = dequeue(queue);
        print_PCB(p);
        enqueue(queue, p);
    }
    printf("\n");
}

int semWait(Mutex *m, PCB *p)
{
    if (m->value == one)
    {
        m->ownerID = p->process_id;
        m->value = zero;
        return 0;
    }
    else
    {

        p->state = BLOCKED;
        enqueue(&(m->queue), p);
        enqueue(&generalBlockedQueue, p);

        printf("Proccess id %i blocked\n", p->process_id);
        printf("Ready queue 1 content:\n");
        print_queue(&readyQueues[0]);
        printf("Ready queue 2 content:\n");
        print_queue(&readyQueues[1]);
        printf("Ready queue 3 content:\n");
        print_queue(&readyQueues[2]);
        printf("Ready queue 4 content:\n");
        print_queue(&readyQueues[3]);
        printf("Blocked queue content:\n");
        print_queue(&generalBlockedQueue);
        return 1;
    }
}

void semSignal(Mutex *m, PCB *p)
{
    if (m->ownerID == p->process_id)
    {
        if ((&(m->queue))->size == 0)
            m->value = one;
        else
        {
            PCB *p2 = dequeue(&(m->queue));
            p2->state = READY;
            enqueue(&readyQueues[p2->priority - 1], p2);

            int size = (&generalBlockedQueue)->size;
            for (int i = 0; i < size; i++)
            {
                PCB *p3 = dequeue((&generalBlockedQueue));
                if (p3->process_id == p2->process_id)
                {
                    break;
                }
                else
                {
                    enqueue((&generalBlockedQueue), p3);
                }
            }

            m->ownerID = p2->process_id;
            printf("Proccess id %i unblocked\n", p2->process_id);
            printf("Ready queue 1 content:\n");
            print_queue(&readyQueues[0]);
            printf("Ready queue 2 content:\n");
            print_queue(&readyQueues[1]);
            printf("Ready queue 3 content:\n");
            print_queue(&readyQueues[2]);
            printf("Ready queue 4 content:\n");
            print_queue(&readyQueues[3]);
            printf("Blocked queue content:\n");
            print_queue(&generalBlockedQueue);
        }
    }
}

void initialize_mutex(Mutex *mutex)
{
    mutex->value = one;
    (&(mutex->queue))->head = 0;
    (&(mutex->queue))->tail = 0;
    (&(mutex->queue))->size = 0;
}

int read_program_to_memory(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Error opening file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    int start = used;
    char buffer[50];
    while (fgets(buffer, sizeof(buffer), file))
    {
        if (used >= MEMORY_SIZE)
        {
            fprintf(stderr, "Memory overflow.\n");
            exit(EXIT_FAILURE);
        }
        strcpy((&memory[used])->name, "lineOfCode");
        strcpy((&memory[used])->data, buffer);
        used++;
    }
    strcat((&memory[used - 1])->data, "\r\n");

    fclose(file);

    return start; // Returns the start index in memory
}

PCB *create_process(int id, int priority, char *functionName)
{
    PCB *pcb = (PCB *)malloc(sizeof(PCB));
    pcb->memory_start = used;
    strcpy((&memory[used])->name, "PCB");
    memcpy((&memory[used])->data, pcb, sizeof(PCB));
    used++;
    pcb->process_id = id;
    pcb->state = READY;
    pcb->priority = priority;
    int start = read_program_to_memory(functionName);
    pcb->program_counter = start;
    pcb->memory_end = used + 2;

    enqueue(&readyQueues[priority - 1], pcb);
    used += 3;

    return pcb;
}

char *get_variable(PCB *pcb, char *variable)
{
    // Loop through the process's memory range
    for (int i = pcb->memory_start; i < pcb->memory_end; i++)
    {
        if (strcmp((&memory[i])->name, variable) == 0)
        {
            return (&memory[i])->data;
        }
    }

    // If the variable isn't found, return NULL or an appropriate message
    fprintf(stderr, "Error: Variable '%s' not found in memory.\n", variable);
    return NULL;
}

void store_variable(PCB *pcb, char *variable, char *value)
{
    int found = 0;

    // Check if the variable already exists
    for (int i = pcb->memory_start; i < pcb->memory_end; i++)
    {
        if (strcmp((&memory[i])->name, variable) == 0)
        {
            strcpy((&memory[i])->data, value); // Store the new value
            found = 1;                         // Mark that the variable was found
            break;
        }
    }

    // If variable was not found, add it to memory
    if (!found)
    {
        for (int i = pcb->memory_end - 2; i <= pcb->memory_end; i++)
        {
            if (*(int *)(&memory[i])->name == 0)
            {
                strcpy((&memory[i])->name, variable);
                strcpy((&memory[i])->data, value);
                break;
            }
        }
    }
}

void execute_program(PCB *pcb, int quantum)
{
    if (pcb == NULL)
    {
        fprintf(stderr, "Error: Null process control block.\n");
        return;
    }

    printf("\nExecuting program for process id=%d\n", pcb->process_id);

    printf("Ready queue 1 content:\n");
    print_queue(&readyQueues[0]);
    printf("Ready queue 2 content:\n");
    print_queue(&readyQueues[1]);
    printf("Ready queue 3 content:\n");
    print_queue(&readyQueues[2]);
    printf("Ready queue 4 content:\n");
    print_queue(&readyQueues[3]);
    printf("Blocked queue content:\n");
    print_queue(&generalBlockedQueue);
    pcb->state = RUNNING;
    int pc = pcb->program_counter;

    int startingCycle = cycle;

    char *instruction = malloc(50 * sizeof(char));
    while (pc < pcb->memory_end)
    {

        if ((cycle - startingCycle) == quantum)
        {
            int priority = pcb->priority;
            pcb->state = READY;
            if (priority == 4)
            {
                enqueue(&readyQueues[priority - 1], pcb);
            }
            else
            {
                pcb->priority = pcb->priority + 1;
                enqueue(&readyQueues[priority], pcb);
            }
            return;
        }

        if (strcmp((&memory[pc])->name, "lineOfCode") == 0)
        {

            strcpy(instruction, (&memory[pc])->data);

            if (instruction == NULL)
            {
                fprintf(stderr, "Error: Null instruction at pc=%d\n", pc);
                break;
            }

            printf("Processing instruction: %s", instruction);

            char *token = strtok(instruction, " ");

            if (strcmp(token, "print") == 0)
            {

                token = strtok(NULL, "\r");
                printf("Output: %s\n", get_variable(pcb, token));
            }
            else if (strcmp(token, "assign") == 0)
            {

                printf("Entered assign \n");
                char *variable = strtok(NULL, " "); // The variable name
                char *value = strtok(NULL, "\r");   // The value to assign

                char *valueIfReadFile = strtok(value, " ");

                printf("variable: %s\n", variable);
                printf("value: %s\n", value);

                if (variable == NULL || value == NULL)
                {
                    fprintf(stderr, "Error: Null variable or value after 'assign' at pc=%d\n", pc);
                    break;
                }

                // If the value is "input", we ask for user input
                if (strcmp(value, "input") == 0)
                {
                    printf("Please enter a value: ");
                    char user_input[100]; // Adjust the size based on expected input
                    if (fgets(user_input, sizeof(user_input), stdin) == NULL)
                    {
                        fprintf(stderr, "Error: Failed to read user input.\n");
                    }
                    else
                    {
                        user_input[strcspn(user_input, "\n")] = '\0'; // Remove newline
                        store_variable(pcb, variable, user_input);    // Store variable
                    }
                }
                else if (strcmp(valueIfReadFile, "readFile") == 0)
                {
                    char *filename = strtok(NULL, "\r");
                    char *fileNameValRead = get_variable(pcb, filename);

                    FILE *file = fopen(fileNameValRead, "r");
                    if (file)
                    {
                        char buffer[100];
                        while (fgets(buffer, sizeof(buffer), file))
                        {
                            // printf("%s\n", buffer);
                        }
                        fclose(file);
                        store_variable(pcb, variable, buffer);
                    }
                }
                else
                {
                    store_variable(pcb, variable, value); // Store variable with given value
                }
            }
            else if (strcmp(token, "semWait") == 0)
            {
                char *resource = strtok(NULL, "\r");
                int wait = 0;

                if (strcmp(resource, "userOutput") == 0)
                {
                    wait = semWait(&outputMutex, pcb);
                }
                else if (strcmp(resource, "userInput") == 0)
                {
                    wait = semWait(&inputMutex, pcb);
                }
                else if (strcmp(resource, "file") == 0)
                {
                    wait = semWait(&fileMutex, pcb);
                }

                if (wait)
                {
                    cycle++;
                    if ((cycle - startingCycle) == quantum)
                    {
                        if ((pcb->priority < 4))
                        {
                            pcb->priority = pcb->priority + 1;
                        }
                    }
                    pc++;
                    pcb->program_counter = pc; // Move to the next instruction
                    return;
                }
            }
            else if (strcmp(token, "semSignal") == 0)
            {
                char *resource = strtok(NULL, "\r");

                if (strcmp(resource, "userOutput") == 0)
                {
                    semSignal(&outputMutex, pcb);
                }
                else if (strcmp(resource, "userInput") == 0)
                {
                    semSignal(&inputMutex, pcb);
                }
                else if (strcmp(resource, "file") == 0)
                {
                    semSignal(&fileMutex, pcb);
                }
            }
            else if (strcmp(token, "printFromTo") == 0)
            {
                char *startVar = strtok(NULL, " ");
                char *endVar = strtok(NULL, "\r");

                printf("start variable: %s\n", startVar);
                printf("end variable: %s\n", endVar);

                char *start_value = malloc(10 * sizeof(char));
                char *end_value = malloc(10 * sizeof(char));
                strcpy(start_value, get_variable(pcb, startVar));
                strcpy(end_value, get_variable(pcb, endVar));

                if (start_value == NULL || end_value == NULL)
                {
                    fprintf(stderr, "Error: Variable(s) not found for 'printFromTo'.\n");
                    return; // Handle variable not found
                }

                int start = atoi(start_value);
                int end = atoi(end_value);

                printf("The start value is: %d\n", start);
                printf("The end value is: %d\n", end);

                int i;

                if (start <= end)
                {
                    for (i = start; i <= end; ++i)
                    {
                        printf("%d ", i);
                    }
                }
                else
                {
                    for (i = start; i >= end; --i)
                    {
                        printf("%d ", i);
                    }
                }
                printf("\n");

                free(start_value);
                free(end_value);
            }
            else if (strcmp(token, "writeFile") == 0)
            {
                char *filename = strtok(NULL, " "); // Filename
                char *content = strtok(NULL, "\r"); // Variable

                // char *get_variable(PCB *pcb, char *variable)

                char *fileNameVal = get_variable(pcb, filename);
                char *varVal = get_variable(pcb, content);
                // content[strcspn(content, "\r\n")] = 0;

                FILE *file = fopen(fileNameVal, "w");
                if (file)
                {
                    fprintf(file, "%s", varVal);
                    fclose(file);
                }
            }
        }
        else
        {
            break;
        }
        printMemoryPCB(pcb);
        cycle++;
        pc++;
        pcb->program_counter = pc; // Move to the next instruction
    }
    free(instruction);

    pcb->state = TERMINATED; // After execution, the process is terminated
    printf("Process id %i terminated\n", pcb->process_id);
    printf("Ready queue 1 content:\n");
    print_queue(&readyQueues[0]);
    printf("Ready queue 2 content:\n");
    print_queue(&readyQueues[1]);
    printf("Ready queue 3 content:\n");
    print_queue(&readyQueues[2]);
    printf("Ready queue 4 content:\n");
    print_queue(&readyQueues[3]);
    printf("Blocked queue content:\n");
    print_queue(&generalBlockedQueue);
}

void run_scheduler()
{
    while (1)
    {
        if ((&readyQueues[0])->size != 0)
        {
            PCB *p = dequeue((&readyQueues[0]));
            execute_program(p, 1);
        }
        else if ((&readyQueues[1])->size != 0)
        {
            PCB *p = dequeue((&readyQueues[1]));
            execute_program(p, 2);
        }
        else if ((&readyQueues[2])->size != 0)
        {
            PCB *p = dequeue((&readyQueues[2]));
            execute_program(p, 3);
        }
        else if ((&readyQueues[3])->size != 0)
        {
            PCB *p = dequeue((&readyQueues[3]));
            execute_program(p, 4);
        }
        else
        {
            break;
        }
    }
}

// helper function for print memory
void printMemoryPCB(PCB *pcb)
{

    printf("Memory print starts here\n");
    int start = pcb->memory_start;
    int end = pcb->memory_end;
    print_PCB(pcb);
    printf("program lines start from memory index %d to memory index %d\n", start + 1, end - 3);

    char *memoryData = malloc(100);

    for (int i = start + 1; i <= end - 3; i++)
    {
        strcpy(memoryData, (char *)(&memory[i])->data);
        memoryData[strcspn(memoryData, "\r\n")] = 0;
        if (i == end - 3)
        {
            printf("Index %d: %s\n", i, memoryData);
            break;
        }
        printf("Index %d: %s, ", i, memoryData);
    }
    free(memoryData);

    char *var1 = (&memory[pcb->memory_end - 2])->name;
    char *var2 = (&memory[pcb->memory_end - 1])->name;
    char *var3 = (&memory[pcb->memory_end])->name;

    if (strlen(var1) == 0)
    {
        printf("variable space 1 is unused\n");
    }
    else
    {
        printf("variable %s: %s\n", var1, get_variable(pcb, var1));
    }

    if (strlen(var2) == 0)
    {
        printf("variable space 2 is unused\n");
    }
    else
    {
        printf("variable %s: %s\n", var2, get_variable(pcb, var2));
    }

    if (strlen(var3) == 0)
    {
        printf("variable space 3 is unused\n");
    }
    else
    {
        printf("variable %s: %s\n", var3, get_variable(pcb, var3));
    }
}

int main()
{
    used = 0;
    cycle = 0;

    memory = (Element *)malloc(MEMORY_SIZE * sizeof(Element));

    for (int i = 0; i < MEMORY_SIZE; i++)
    {
        (&memory[i])->name = (char *)malloc(100 * sizeof(char));
        (&memory[i])->data = malloc(100);
    }

    initialize_mutex(&inputMutex);
    initialize_mutex(&outputMutex);
    initialize_mutex(&fileMutex);

    for (int i = 0; i < 4; i++)
    {
        (&readyQueues[i])->head = 0;
        (&readyQueues[i])->tail = 0;
        (&readyQueues[i])->size = 0;
    }

    (&generalBlockedQueue)->head = 0;
    (&generalBlockedQueue)->tail = 0;
    (&generalBlockedQueue)->size = 0;

    create_process(1, 1, "Program_1.txt");
    create_process(2, 1, "Program_2.txt");
    create_process(3, 1, "Program_3.txt");

    run_scheduler();

    for (int i = 0; i < MEMORY_SIZE; i++)
    {
        free((&memory[i])->data);
        free((&memory[i])->name);
    }

    return 0;
}