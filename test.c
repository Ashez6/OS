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
    int pid = 3;
    char str[20];
    char str1[20] = "a";

    sprintf(str + strlen(str), "%d", pid, str1);

    printf("%s", str);
    return 0;
}