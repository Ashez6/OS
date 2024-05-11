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

void *memory[60][2];
int used = 0;
Mutex inputMutex;
Mutex outputMutex;
Mutex fileMutex;
Queue generalBlockedQueue;
Queue readyQueues[4];
int cycle;

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

void semWait(Mutex *m, PCB *p)
{
    if (m->value == one)
    {
        m->ownerID = p->process_id;
        m->value = zero;
    }
    else
    {
        p->state = BLOCKED;
        enqueue(&(m->queue), p);
        enqueue(&generalBlockedQueue, p);
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
            p->state = READY;
            dequeue(&generalBlockedQueue);
            PCB *p2 = dequeue(&(m->queue));
            m->ownerID = p2->process_id;
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
        strcpy((char *)memory[used][0], "lineOfCode");
        strcpy((char *)memory[used][1], buffer);
        used++;
    }
    strcat((char *)memory[used - 1][1], "\r\n");

    fclose(file);

    return start; // Returns the start index in memory
}

PCB *create_process(int id, int priority, char *functionName)
{
    PCB *pcb = (PCB *)malloc(sizeof(PCB));
    pcb->memory_start = used;
    strcpy(memory[used][0], "PCB");
    memcpy(memory[used][1], pcb, sizeof(PCB));
    used++;
    pcb->process_id = id;
    pcb->state = NEW;
    pcb->priority = priority;
    int start = read_program_to_memory(functionName);
    pcb->program_counter = start;
    pcb->memory_end = used + 2;

    return pcb;
}

char *get_variable(PCB *pcb, char *variable)
{
    // Loop through the process's memory range
    for (int i = pcb->memory_start; i < pcb->memory_end; i++)
    {
        if (strcmp(memory[i][0], variable) == 0)
        {
            return memory[i][1];
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
        if (strcmp(memory[i][0], variable) == 0)
        {
            strcpy(memory[i][1], value); // Store the new value
            found = 1;                   // Mark that the variable was found
            break;
        }
    }

    // If variable was not found, add it to memory
    if (!found)
    {
        for (int i = pcb->memory_end - 2; i <= pcb->memory_end; i++)
        {
            // printf("mem location %i",((char*)memory[i][0]);
            if (*(int *)memory[i][0] == 0)
            {
                strcpy(memory[i][0], variable);
                strcpy(memory[i][1], value);
                break;
            }
        }
    }
}

void execute_program(PCB *pcb)
{
    if (pcb == NULL)
    {
        fprintf(stderr, "Error: Null process control block.\n");
        return;
    }

    printf("Executing program for process id=%d\n", pcb->process_id);

    int pc = pcb->program_counter;

    char *instruction = malloc(50 * sizeof(char));
    while (pc < pcb->memory_end)
    {
        if (strcmp(((char *)memory[pc][0]), "lineOfCode") == 0)
        {

            strcpy(instruction, memory[pc][1]);

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

                semWait(&outputMutex, pcb);
                printf("Output: %s\n", get_variable(pcb, token));
                semSignal(&outputMutex, pcb);
            }
            else if (strcmp(token, "assign") == 0)
            {

                printf("Entered assign \n");
                char *variable = strtok(NULL, " "); // The variable name
                char *value = strtok(NULL, "\r");   // The value to assign

                char *valueIfReadFile = strtok(value, " ");

                printf("variable: %s\n", variable);
                printf("value: %s\n", value);
                printf("value: %s\n", valueIfReadFile);

                if (variable == NULL || value == NULL)
                {
                    fprintf(stderr, "Error: Null variable or value after 'assign' at pc=%d\n", pc);
                    break;
                }

                // If the value is "input", we ask for user input
                if (strcmp(value, "input") == 0)
                {
                    semWait(&inputMutex, pcb); // Locking user input
                    printf("Please enter a value: ");
                    char user_input[50]; // Adjust the size based on expected input
                    if (fgets(user_input, sizeof(user_input), stdin) == NULL)
                    {
                        fprintf(stderr, "Error: Failed to read user input.\n");
                    }
                    else
                    {
                        user_input[strcspn(user_input, "\n")] = '\0'; // Remove newline
                        store_variable(pcb, variable, user_input);    // Store variable
                    }

                    semSignal(&inputMutex, pcb); // Unlocking user input
                }
                else if (strcmp(valueIfReadFile, "readFile") == 0)
                {
                    char *filename = strtok(NULL, "\r");
                    char *fileNameValRead = get_variable(pcb, filename);

                    semWait(&fileMutex, pcb);
                    FILE *file = fopen(fileNameValRead, "r");
                    if (file)
                    {
                        char buffer[100];
                        while (fgets(buffer, sizeof(buffer), file))
                        {
                            printf("%s", buffer);
                        }
                        fclose(file);
                        store_variable(pcb, variable, buffer);
                    }
                    semSignal(&fileMutex, pcb);
                }
                else
                {
                    store_variable(pcb, variable, value); // Store variable with given value
                }
            }
            else if (strcmp(token, "semWait") == 0)
            {
                char *resource = strtok(NULL, "\r");

                if (strcmp(resource, "userOutput") == 0)
                {
                    semWait(&outputMutex, pcb);
                }
                else if (strcmp(resource, "userInput") == 0)
                {
                    semWait(&inputMutex, pcb);
                }
                else if (strcmp(resource, "file") == 0)
                {
                    semWait(&fileMutex, pcb);
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

                printf("The start value is :%d\n", start);
                printf("The end value is:%d\n", end);

                semWait(&outputMutex, pcb);

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
                semSignal(&outputMutex, pcb);
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

                semWait(&fileMutex, pcb);
                FILE *file = fopen(fileNameVal, "w");
                if (file)
                {
                    fprintf(file, "%s", varVal);
                    fclose(file);
                }
                semSignal(&fileMutex, pcb);
            }
        }
        else
        {
            break;
        }

        pc++; // Move to the next instruction
    }
    free(instruction);

    pcb->state = TERMINATED; // After execution, the process is terminated
}

int main()
{
    used = 0;
    cycle = 0;
    for (int i = 0; i < 60; i++)
    {
        memory[i][0] = malloc(sizeof(void *));
        memory[i][1] = malloc(sizeof(void *));
    }

    initialize_mutex(&inputMutex);
    initialize_mutex(&outputMutex);
    initialize_mutex(&fileMutex);

    (&generalBlockedQueue)->head = 0;
    (&generalBlockedQueue)->tail = 0;
    (&generalBlockedQueue)->size = 0;

    PCB *p = create_process(1, 1, "Program_3.txt");
    execute_program(p);

    // Free dynamically allocated memory
    for (int i = 0; i < 60; i++)
    {
        free(memory[i][0]);
        free(memory[i][1]);
    }

    return 0;
}