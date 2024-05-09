#include <stdio.h>
#include <stdlib.h>

#define MEM_SIZE = 60;

char *memory[MEM_SIZE];

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

int main(int argc, char const *argv[])
{
    return 0;
}
