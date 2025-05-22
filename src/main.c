#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))

char *pathLookup(char *name)
{
    static char fullPath[256];
    char *path = getenv("PATH");
    for (;;)
    {
        char *separator = strchr(path, ':');
        int pathLen = separator != NULL ? separator - path : strlen(path);
        if (pathLen == 0)
        {
            break;
        }
        snprintf(fullPath, ARRAY_LENGTH(fullPath), "%.*s/%s", pathLen, path, name);
        if (access(fullPath, F_OK) == 0)
        {
            return fullPath;
        }
        if (separator == NULL)
        {
            break;
        }
        path = separator + 1;
    }
    return NULL;
}

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
    char *fullPath = pathLookup(arg);
    if (fullPath != NULL)
    {
        printf("%s is %s\n", arg, fullPath);
        return;
    }
    printf("%s: not found\n", arg);
}

char **splitCommandLine(char *input, int *argCount)
{
    *argCount = 0;
    int capacity = 0;
    char **result = NULL;
    char *current = strtok(input, " \t\n\r");
    while (current != NULL)
    {
        if (capacity < *argCount + 1)
        {
            capacity = capacity == 0 ? 8 : capacity * 2;
            result = realloc(result, sizeof(char *) * capacity);
            if (result == NULL)
            {
                exit(EXIT_FAILURE);
            }
        }
        int length = strlen(current) + 1;
        char *arg = malloc(sizeof(char) * length);
        snprintf(arg, length, "%s", current);
        result[*argCount] = arg;
        *argCount += 1;
        current = strtok(NULL, " \t\n\r");
    }
    return result;
}

void freeArrayAndElements(char **array, int size)
{
    for (int i = 0; i < size; i++)
    {
        free(array[i]);
    }
    free(array);
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
        int argCount = 0;
        char **args = splitCommandLine(input, &argCount);
        if (argCount == 0)
        {
            continue;
        }
        char *cmd = args[0];
        for (int i = 0; i < argCount; i++)
        {
            printf("%d: %s\n", i, args[i]);
        }
        if (strcmp(cmd, "exit") == 0)
        {
            break;
        }
        else if (strcmp(cmd, "echo") == 0)
        {
            for (int i = 1; i < argCount; i++)
            {
                printf("%s ", args[i]);
            }
            printf("\n");
        }
        else if (strcmp(cmd, "type") == 0)
        {
            if (argCount > 1)
            {
                cmdType(args[1]);
            }
        }
        else
        {
            printf("%s: command not found\n", cmd);
        }
        freeArrayAndElements(args, argCount);
    }
    return 0;
}
