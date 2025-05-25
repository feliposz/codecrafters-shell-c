#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))
#define MAX_PATH_LENGTH 1024
#define MAX_INPUT 1024

char *pathLookup(char *name)
{
    char fullPath[MAX_PATH_LENGTH];
    char *path = getenv("PATH");
    if (path == NULL)
    {
        return NULL;
    }
    for (;;)
    {
        char *separator = strchr(path, ':');
        int pathLen = separator != NULL ? separator - path : strlen(path);
        if (pathLen == 0)
        {
            break;
        }
        int fullPathLen = snprintf(fullPath, MAX_PATH_LENGTH, "%.*s/%s", pathLen, path, name);
        if (access(fullPath, F_OK) == 0)
        {
            char *result = malloc(fullPathLen + 1);
            if (result == NULL)
            {
                perror("malloc");
                return NULL;
            }
            snprintf(result, fullPathLen + 1, "%s", fullPath);
            return result;
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
        "pwd",
        "cd",
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
        free(fullPath);
        return;
    }
    printf("%s: not found\n", arg);
}

char **splitCommandLine(char *input)
{
    int count = 0;
    int capacity = 0;
    char **result = NULL;
    char token[MAX_INPUT];
    int length = 0;
    char *cur = input;
    for (;;)
    {
        bool addToken = false;
        if (cur[0] == '\'')
        {
            do
            {
                cur++; // opening '
                while (cur[0] != '\'' && cur[0] != '\0')
                {
                    token[length++] = cur[0];
                    cur++;
                }
                if (cur[0] == '\0')
                {
                    printf("unmatched '\n");
                    return NULL;
                }
                cur++; // closing '
            } while (cur[0] == '\''); // handle concatenation
        }
        if (isspace(cur[0]))
        {
            addToken = true;
            while (isspace(cur[0]))
            {
                cur++;
            }
        }
        if (addToken || cur[0] == '\0')
        {
            if (capacity < count + 1)
            {
                capacity = capacity == 0 ? 8 : capacity * 2;
                result = realloc(result, sizeof(char *) * capacity);
                if (result == NULL)
                {
                    exit(EXIT_FAILURE);
                }
            }
            char *arg = malloc(length + 1);
            snprintf(arg, length + 1, "%s", token);
            result[count++] = arg;
            length = 0;
            if (cur[0] == '\0')
            {
                break;
            }
        }
        else
        {
            token[length++] = cur[0];
            cur++;
        }
    }
    if (count == 0)
    {
        return NULL;
    }
    if (capacity < count + 1)
    {
        capacity++;
        result = realloc(result, sizeof(char *) * capacity);
        if (result == NULL)
        {
            exit(EXIT_FAILURE);
        }
    }
    result[count++] = NULL;
    return result;
}

void freeArrayAndElements(char **array)
{
    for (int i = 0; array[i] != NULL; i++)
    {
        free(array[i]);
    }
    free(array);
}

void runCmd(char *cmd, char **args)
{
    int pid = fork();
    if (pid == 0)
    {
        execv(cmd, args);
        perror("execv");
        _exit(EXIT_FAILURE); // child should not return
    }
    else if (pid > 0)
    {
        int status;
        waitpid(pid, &status, WUNTRACED);
    }
    else
    {
        perror("fork");
    }
}

int main(int argc, char *argv[])
{
    // Flush after every printf
    setbuf(stdout, NULL);

    char input[MAX_INPUT];
    for (;;)
    {
        printf("$ ");
        fgets(input, MAX_INPUT, stdin);
        if (feof(stdin))
        {
            break;
        }
        char **args = splitCommandLine(input);
        if (args == NULL)
        {
            continue;
        }
        char *cmd = args[0];
        // for (int i = 0; args[i] != NULL; i++)
        // {
        //     printf("%d: %s\n", i, args[i]);
        // }
        if (strcmp(cmd, "exit") == 0)
        {
            break;
        }
        else if (strcmp(cmd, "echo") == 0)
        {
            for (int i = 1; args[i] != NULL; i++)
            {
                printf("%s ", args[i]);
            }
            printf("\n");
        }
        else if (strcmp(cmd, "type") == 0)
        {
            if (args[1] != NULL)
            {
                cmdType(args[1]);
            }
        }
        else if (strcmp(cmd, "pwd") == 0)
        {
            char currentDir[MAX_PATH_LENGTH];
            getcwd(currentDir, MAX_PATH_LENGTH);
            printf("%s\n", currentDir);
        }
        else if (strcmp(cmd, "cd") == 0)
        {
            char *path = args[1];
            if (path != NULL)
            {
                if (strcmp("~", path) == 0)
                {
                    char *envHome = getenv("HOME");
                    path = envHome != NULL ? envHome : path;
                }
                if (chdir(path) != 0)
                {
                    printf("cd: %s: %s\n", path, strerror(errno));
                }
            }
        }
        else
        {
            char *fullPath = pathLookup(cmd);
            if (fullPath != NULL)
            {
                runCmd(fullPath, args);
                free(fullPath);
            }
            else
            {
                printf("%s: command not found\n", cmd);
            }
        }
        freeArrayAndElements(args);
    }
    return 0;
}
