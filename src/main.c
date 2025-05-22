#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))

void cmdType(char *arg)
{
    char *builtins[] = {
        "exit",
        "echo",
        "type",
    };
    for (int i = 0; i < ARRAY_LENGTH(builtins); i++)
    {
        if (strcmp(arg, builtins[i]) == 0)
        {
            printf("%s is a shell builtin\n", arg);
            return;
        }
    }
    printf("%s: not found\n", arg);
}

int main(int argc, char *argv[])
{
    // Flush after every printf
    setbuf(stdout, NULL);

    char input[100];
    for (;;)
    {
        printf("$ ");
        fgets(input, 100, stdin);
        if (feof(stdin))
        {
            break;
        }
        input[strlen(input) - 1] = '\0';
        char *cmd = strtok(input, " \t");
        if (strcmp(cmd, "exit") == 0)
        {
            break;
        }
        else if (strcmp(cmd, "echo") == 0)
        {
            char *arg = strtok(NULL, " \t");
            while (arg != NULL)
            {
                printf("%s ", arg);
                arg = strtok(NULL, " \t");
            }
            printf("\n");
        }
        else if (strcmp(cmd, "type") == 0)
        {
            char *arg = strtok(NULL, " \t");
            if (arg != NULL)
            {
                cmdType(arg);
            }
        }
        else
        {
            printf("%s: command not found\n", cmd);
        }
    }
    return 0;
}
