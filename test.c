#include <stdio.h>
#include <string.h>

void removeNewline(char *str)
{
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n')
    {
        str[len - 1] = '\0'; // Remove trailing newline
    }
}

int main()
{
    char myString[] = "b\n";
    printf("Original string: %s\n", myString);

    myString[strcspn(myString, "\r\n")] = 0;

    printf("Modified string: %s\n", myString);

    return 0;
}