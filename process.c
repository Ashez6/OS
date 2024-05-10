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
    if (mutex == NULL)
    {
        fprintf(stderr, "Error: Null mutex during semWait.\n");
        exit(EXIT_FAILURE);
    }
    printf("semWait on mutex, is_locked=%d\n", mutex->is_locked);

    if (mutex->is_locked)
    {
        printf("Mutex is locked, adding PCB to blocked queue.\n");
        pcb->state = BLOCKED;
        enqueue(&mutex->blocked_queue, pcb);
        enqueue(general_blocked_queue, pcb);
    }
    else
    {
        mutex->is_locked = true;
        printf("Mutex is now locked.\n");
    }
}

// Function to release a mutex (semWait)

void semSignal(Mutex *mutex, ProcessQueue **ready_queues, int max_priority)
{
    if (mutex == NULL)
    {
        fprintf(stderr, "Error: Null mutex during semSignal.\n");
        exit(EXIT_FAILURE);
    }

    printf("semSignal on mutex, is_locked=%d\n", mutex->is_locked);

    if (mutex->blocked_queue.front != mutex->blocked_queue.rear)
    {
        printf("Unblocking process from mutex.\n");
        ProcessControlBlock *pcb = dequeue(&mutex->blocked_queue);
        mutex->is_locked = false;

        enqueue(ready_queues[pcb->priority - 1], pcb);
        pcb->state = READY;
    }
    else
    {
        mutex->is_locked = false;
    }
}

// Function to run the scheduler
void run_scheduler(ProcessQueue **ready_queues, int *current_priority, int *quantum)
{
    for (int i = 0; i < MAX_QUEUES; i++)
    {
        if (ready_queues[i]->front != ready_queues[i]->rear)
        {
            *current_priority = i + 1; // Set to the queue with processes

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

            return; // Return as we've found a queue with a process
        }
    }

    *current_priority = 0; // No processes to execute
    *quantum = 0;          // No quantum time
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
    char buffer[100]; // Increase the buffer size to accommodate longer lines

    while (fgets(buffer, sizeof(buffer), file))
    {
        // Strip any line-ending characters
        buffer[strcspn(buffer, "\r\n")] = 0;

        if (memory->used >= MEMORY_SIZE)
        {
            fprintf(stderr, "Memory overflow.\n");
            exit(EXIT_FAILURE);
        }

        strcpy(memory->data[memory->used], buffer); // Store the complete line in memory
        memory->used++;
    }

    fclose(file);
    return start; // Returns the start index in memory
}

void store_variable_in_memory(Memory *memory, ProcessControlBlock *pcb, const char *variable, const char *value)
{
    if (memory == NULL || pcb == NULL || variable == NULL || value == NULL)
    {
        fprintf(stderr, "Error: Invalid parameter passed to store_variable_in_memory.\n");
        return;
    }

    int found = 0;

    // Check if the variable already exists
    for (int i = pcb->memory_start; i < pcb->memory_end; i++)
    {
        if (strcmp(memory->data[i], variable) == 0)
        {
            strcpy(memory->data[i + 1], value); // Store the new value
            found = 1;                          // Mark that the variable was found
            break;
        }
    }

    // If variable was not found, add it to memory
    if (!found)
    {
        if (pcb->memory_end + 2 > MEMORY_SIZE)
        {
            fprintf(stderr, "Error: Memory overflow while storing variable '%s'.\n", variable);
            return; // Handle overflow
        }

        // Add the variable name and its value
        strcpy(memory->data[pcb->memory_end], variable);
        strcpy(memory->data[pcb->memory_end + 1], value);

        pcb->memory_end += 2; // Increment memory_end to account for the new variable and its value
    }
}

const char *get_variable_value(Memory *memory, ProcessControlBlock *pcb, const char *variable)
{
    if (memory == NULL || pcb == NULL || variable == NULL)
    {
        fprintf(stderr, "Error: Invalid parameter passed to get_variable_value.\n");
        return NULL;
    }

    // Loop through the process's memory range
    for (int i = pcb->memory_start; i < pcb->memory_end; i++)
    {
        if (strcmp(memory->data[i], variable) == 0)
        {
            if (i + 1 < pcb->memory_end)
            {                               // Ensure there's a value after the variable name
                return memory->data[i + 1]; // Return the associated value
            }
            else
            {
                fprintf(stderr, "Error: Variable '%s' found, but no associated value.\n", variable);
                return NULL;
            }
        }
    }

    // If the variable isn't found, return NULL or an appropriate message
    fprintf(stderr, "Error: Variable '%s' not found in memory.\n", variable);
    return NULL;
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
            // token[strcspn(token, "\r\n")] = 0;

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
            printf("Entered assign \n");
            char *variable = strtok(NULL, " "); // The variable name
            char *value = strtok(NULL, " ");    // The value to assign

            printf("variable: %s\n", variable);

            value[strcspn(value, "\r\n")] = 0;

            printf("value: %s\n", value);

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
                if (fgets(user_input, sizeof(user_input), stdin) == NULL)
                {
                    fprintf(stderr, "Error: Failed to read user input.\n");
                }
                else
                {
                    user_input[strcspn(user_input, "\n")] = '\0';                // Remove newline
                    store_variable_in_memory(memory, pcb, variable, user_input); // Store variable
                }

                semSignal(user_input_mutex, ready_queues, 4); // Unlocking user input
            }
            else
            {
                store_variable_in_memory(memory, pcb, variable, value); // Store variable with given value
            }
        }
        else if (strcmp(token, "semWait") == 0)
        {
            char *resource = strtok(NULL, "\r");

            printf("h%sh\n", resource);

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
            resource[strcspn(resource, "\r\n")] = 0;
            printf("h%sh\n", resource);

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
            char *startVar = strtok(NULL, " ");
            char *endVar = strtok(NULL, " ");
            endVar[strcspn(endVar, "\r\n")] = 0;

            printf("start variable: %s\n", startVar);
            printf("end variable: %s\n", endVar);

            const char *start_value = get_variable_value(memory, pcb, startVar);
            const char *end_value = get_variable_value(memory, pcb, endVar);

            if (start_value == NULL || end_value == NULL)
            {
                fprintf(stderr, "Error: Variable(s) not found for 'printFromTo'.\n");
                return; // Handle variable not found
            }

            int start = atoi(start_value);
            int end = atoi(end_value);

            printf("The start value is :%d\n", start);
            printf("The end value is:%d\n", end);

            semWait(user_output_mutex, pcb, general_blocked_queue);

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
            semSignal(user_output_mutex, ready_queues, 4);
        }
        else if (strcmp(token, "writeFile") == 0)
        {
            char *filename = strtok(NULL, " ");
            char *content = strtok(NULL, " ");

            // content[strcspn(content, "\r\n")] = 0;

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
            // filename[strcspn(filename, "\r\n")] = 0;

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
        else
        {
            printf("No processes at current priority level, exiting loop.\n");
            break; // Exit the loop if there's no valid priority
        }
    }

    // Free allocated memory for each queue
    for (int i = 0; i < 4; i++)
    {
        free(ready_queues[i]);
    }

    return 0;
}