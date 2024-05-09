#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdbool.h>
#include <ctype.h>

// Define constants
#define MEMORY_SIZE 60
#define MAX_PROCESSES 10
#define MAX_QUEUES 4

// Quantum times for different levels
#define QUANTUM_LVL_1 1
#define QUANTUM_LVL_2 2
#define QUANTUM_LVL_3 4
#define QUANTUM_LVL_4 8

// Enum to represent process state
typedef enum
{
    NEW,
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
} ProcessState;

// Struct for Process Control Block
typedef struct
{
    int process_id;
    ProcessState state;
    int priority;
    int program_counter;
    int memory_start;
    int memory_end;
} ProcessControlBlock;

// Struct for Memory
typedef struct
{
    char data[MEMORY_SIZE][50]; // Each memory word can store 50 characters
    int used;                   // How much memory is used
} Memory;

// Define queues for the ready queue with multiple levels of priority
typedef struct
{
    int front;
    int rear;
    int size;
    ProcessControlBlock *queue[MAX_PROCESSES];
} ProcessQueue;

// Helper function to create a new PCB
ProcessControlBlock *create_process(int id, int priority, int memory_start, int memory_end)
{
    ProcessControlBlock *pcb = (ProcessControlBlock *)malloc(sizeof(ProcessControlBlock));
    if (pcb == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    pcb->process_id = id;
    pcb->state = NEW;
    pcb->priority = priority;
    pcb->program_counter = memory_start; // Assume the program starts here
    pcb->memory_start = memory_start;
    pcb->memory_end = memory_end;

    return pcb;
}

// Helper function to initialize memory
void initialize_memory(Memory *memory)
{
    memory->used = 0;
    memset(memory->data, 0, sizeof(memory->data));
}

// Enqueue function
void enqueue(ProcessQueue *queue, ProcessControlBlock *pcb)
{
    if (queue == NULL || pcb == NULL)
    {
        fprintf(stderr, "Error: Null pointer during enqueue.\n");
        return;
    }
    if (queue->size >= MAX_PROCESSES)
    {
        fprintf(stderr, "Error: Queue is full.\n");
        return;
    }
    queue->queue[queue->rear] = pcb;
    queue->rear = (queue->rear + 1) % MAX_PROCESSES;
    queue->size++;
}

// Dequeue function
ProcessControlBlock *dequeue(ProcessQueue *queue)
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
    ProcessControlBlock *pcb = queue->queue[queue->front];
    queue->front = (queue->front + 1) % MAX_PROCESSES;
    queue->size--;
    return pcb;
}

// Initialize a queue
void initialize_queue(ProcessQueue *queue)
{
    if (queue == NULL)
    {
        fprintf(stderr, "Error: Null queue pointer.\n");
        exit(EXIT_FAILURE);
    }
    queue->front = 0;
    queue->rear = 0;
    queue->size = 0;
}

// Mutex structure for mutual exclusion
typedef struct
{
    bool is_locked;
    ProcessQueue blocked_queue;
} Mutex;

// Function to initialize a mutex
void initialize_mutex(Mutex *mutex)
{
    mutex->is_locked = false;
    initialize_queue(&mutex->blocked_queue);
}

// Function to acquire a mutex (semWait)
void semWait(Mutex *mutex, ProcessControlBlock *pcb, ProcessQueue *general_blocked_queue)
{
    if (mutex->is_locked)
    {
        pcb->state = BLOCKED;
        enqueue(&mutex->blocked_queue, pcb);
        enqueue(general_blocked_queue, pcb);
    }
    else
    {
        mutex->is_locked = true;
    }
}

// Function to release a mutex (semSignal)
void semSignal(Mutex *mutex, ProcessQueue *ready_queues[], int max_priority)
{
    if (mutex->blocked_queue.front != mutex->blocked_queue.rear)
    {
        ProcessControlBlock *pcb = dequeue(&mutex->blocked_queue);
        mutex->is_locked = false;

        // Add the process to the correct ready queue based on its priority
        enqueue(ready_queues[pcb->priority - 1], pcb);
        pcb->state = READY;
    }
}

// Function to run the scheduler
void run_scheduler(ProcessQueue *ready_queues[], int *current_priority, int *quantum)
{
    // Loop through the queues from highest to lowest priority
    for (int i = 0; i < MAX_QUEUES; i++)
    {
        if (ready_queues[i]->front != ready_queues[i]->rear)
        {
            *current_priority = i + 1;

            switch (*current_priority)
            {
            case 1:
                *quantum = QUANTUM_LVL_1;
                break;
            case 2:
                *quantum = QUANTUM_LVL_2;
                break;
            case 3:
                *quantum = QUANTUM_LVL_3;
                break;
            case 4:
                *quantum = QUANTUM_LVL_4;
                break;
            }

            return;
        }
    }

    *current_priority = 0; // If no process is in any queue, set to 0
    *quantum = 0;          // No process to execute
}

int read_program_to_memory(const char *filename, Memory *memory)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Error opening file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    int start = memory->used;
    char buffer[50];

    while (fgets(buffer, sizeof(buffer), file))
    {
        if (memory->used >= MEMORY_SIZE)
        {
            fprintf(stderr, "Memory overflow.\n");
            exit(EXIT_FAILURE);
        }
        strcpy(memory->data[memory->used], buffer);
        memory->used++;
    }

    fclose(file);
    return start; // Returns the start index in memory
}

// Function to execute a program based on its instructions
void execute_program(ProcessControlBlock *pcb, Memory *memory, Mutex *user_output_mutex, Mutex *user_input_mutex, Mutex *file_mutex, ProcessQueue *ready_queues[], ProcessQueue *general_blocked_queue)
{
    if (pcb == NULL)
    {
        fprintf(stderr, "Error: Null process control block.\n");
        return;
    }

    printf("Executing program for process id=%d\n", pcb->process_id);

    int pc = pcb->program_counter;

    while (pc < pcb->memory_end)
    {
        char *instruction = memory->data[pc];

        if (instruction == NULL)
        {
            fprintf(stderr, "Error: Null instruction at pc=%d\n", pc);
            break;
        }

        printf("Processing instruction: %s\n", instruction);

        // Tokenize the instruction
        char *token = strtok(instruction, " ");

        if (token == NULL)
        {
            fprintf(stderr, "Error: Null token in instruction at pc=%d\n", pc);
            break;
        }

        if (strcmp(token, "print") == 0)
        {

            token = strtok(NULL, " ");
            if (token == NULL)
            {
                fprintf(stderr, "Error: Null token after 'print' at pc=%d\n", pc);
                break;
            }

            int value = atoi(token);

            semWait(user_output_mutex, pcb, general_blocked_queue);
            printf("Output: %d\n", value);
            semSignal(user_output_mutex, ready_queues, 4);
        }
        else if (strcmp(token, "assign") == 0)
        {
            char *variable = strtok(NULL, " "); // The variable name
            char *value = strtok(NULL, " ");    // The value to assign

            if (variable == NULL || value == NULL)
            {
                fprintf(stderr, "Error: Null variable or value after 'assign' at pc=%d\n", pc);
                break;
            }

            // If the value is "input", we ask for user input
            if (strcmp(value, "input") == 0)
            {
                semWait(user_input_mutex, pcb, general_blocked_queue); // Locking user input
                printf("Please enter a value: ");
                char user_input[50]; // Adjust the size based on expected input
                fgets(user_input, sizeof(user_input), stdin);
                user_input[strcspn(user_input, "\n")] = '\0'; // Remove newline

                // Store the user input in memory
                for (int i = pcb->memory_start; i < pcb->memory_end; i++)
                {
                    if (strcmp(memory->data[i], variable) == 0)
                    {
                        strcpy(memory->data[i], user_input);
                        break;
                    }
                }

                semSignal(user_input_mutex, ready_queues, 4); // Unlocking user input
            }
            else
            {
                // If it's not "input", just assign the given value
                for (int i = pcb->memory_start; i < pcb->memory_end; i++)
                {
                    if (strcmp(memory->data[i], variable) == 0)
                    {
                        strcpy(memory->data[i], value);
                        break;
                    }
                }
            }
        }
        else if (strcmp(token, "semWait") == 0)
        {
            char *resource = strtok(NULL, " ");

            if (strcmp(resource, "userOutput") == 0)
            {
                semWait(user_output_mutex, pcb, general_blocked_queue);
            }
            else if (strcmp(resource, "userInput") == 0)
            {
                semWait(user_input_mutex, pcb, general_blocked_queue);
            }
            else if (strcmp(resource, "file") == 0)
            {
                semWait(file_mutex, pcb, general_blocked_queue);
            }
        }
        else if (strcmp(token, "semSignal") == 0)
        {
            char *resource = strtok(NULL, " ");

            if (strcmp(resource, "userOutput") == 0)
            {
                semSignal(user_output_mutex, ready_queues, 4);
            }
            else if (strcmp(resource, "userInput") == 0)
            {
                semSignal(user_input_mutex, ready_queues, 4);
            }
            else if (strcmp(resource, "file") == 0)
            {
                semSignal(file_mutex, ready_queues, 4);
            }
        }
        else if (strcmp(token, "printFromTo") == 0)
        {
            int start = atoi(strtok(NULL, " "));
            int end = atoi(strtok(NULL, " "));

            semWait(user_output_mutex, pcb, general_blocked_queue);
            for (int i = start; i <= end; i++)
            {
                printf("%d ", i);
            }
            printf("\n");
            semSignal(user_output_mutex, ready_queues, 4);
        }
        else if (strcmp(token, "writeFile") == 0)
        {
            char *filename = strtok(NULL, " ");
            char *content = strtok(NULL, " ");

            semWait(file_mutex, pcb, general_blocked_queue);
            FILE *file = fopen(filename, "w");
            if (file)
            {
                fprintf(file, "%s", content);
                fclose(file);
            }
            semSignal(file_mutex, ready_queues, 4);
        }
        else if (strcmp(token, "readFile") == 0)
        {
            char *filename = strtok(NULL, " ");

            semWait(file_mutex, pcb, general_blocked_queue);
            FILE *file = fopen(filename, "r");
            if (file)
            {
                char buffer[100];
                while (fgets(buffer, sizeof(buffer), file))
                {
                    printf("%s", buffer);
                }
                fclose(file);
            }
            semSignal(file_mutex, ready_queues, 4);
        }

        pc++; // Move to the next instruction
    }

    pcb->state = TERMINATED; // After execution, the process is terminated
}

int main()
{
    // Initialize memory
    Memory memory;
    initialize_memory(&memory);

    // Initialize mutexes for user input/output and file access
    Mutex user_output_mutex;
    Mutex user_input_mutex;
    Mutex file_mutex;

    initialize_mutex(&user_output_mutex);
    initialize_mutex(&user_input_mutex);
    initialize_mutex(&file_mutex);

    // Initialize the ready queues for different priority levels
    ProcessQueue *ready_queues[4];
    for (int i = 0; i < 4; i++)
    {
        ready_queues[i] = malloc(sizeof(ProcessQueue));
        if (ready_queues[i] == NULL)
        {
            fprintf(stderr, "Memory allocation failed for ready queue.\n");
            exit(EXIT_FAILURE);
        }
        initialize_queue(ready_queues[i]);
    }

    // General blocked queue
    ProcessQueue general_blocked_queue;
    initialize_queue(&general_blocked_queue);

    // Read the programs into memory and create processes
    int start_1 = read_program_to_memory("Program_1.txt", &memory);
    int end_1 = memory.used; // Exclusive end boundary

    int start_2 = read_program_to_memory("Program_2.txt", &memory);
    int end_2 = memory.used;

    int start_3 = read_program_to_memory("Program_3.txt", &memory);
    int end_3 = memory.used;

    // Create Process Control Blocks for each program
    ProcessControlBlock *pcb1 = create_process(1, 1, start_1, end_1);
    ProcessControlBlock *pcb2 = create_process(2, 2, start_2, end_2);
    ProcessControlBlock *pcb3 = create_process(3, 3, start_3, end_3);

    // Enqueue the processes in the correct ready queue based on their priority
    enqueue(ready_queues[0], pcb1); // Priority 1
    enqueue(ready_queues[1], pcb2); // Priority 2
    enqueue(ready_queues[2], pcb3); // Priority 3

    // Run the scheduler and execute the processes
    int current_priority = 0;
    int quantum = 0;

    while (current_priority > 0 || ready_queues[current_priority - 1]->front != ready_queues[current_priority - 1]->rear)
    {
        run_scheduler(ready_queues, &current_priority, &quantum);

        if (current_priority > 0)
        {
            // Get the first process from the ready queue of the current priority
            ProcessControlBlock *current_pcb = dequeue(ready_queues[current_priority - 1]);

            current_pcb->state = RUNNING;

            // Execute the process
            execute_program(current_pcb, &memory, &user_output_mutex, &user_input_mutex, &file_mutex, ready_queues, &general_blocked_queue);

            // If it's not terminated, enqueue it back to the ready queue
            if (current_pcb->state != TERMINATED)
            {
                current_pcb->state = READY;
                enqueue(ready_queues[current_priority - 1], current_pcb);
            }
        }
    }

    // Free allocated memory for each queue
    for (int i = 0; i < 4; i++)
    {
        free(ready_queues[i]);
    }

    return 0;
}