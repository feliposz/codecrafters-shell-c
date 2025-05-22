#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        else
        {
            printf("%s: command not found\n", cmd);
        }
    }
    return 0;
}
