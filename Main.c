#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <sys/wait.h>
bool sigue = true;
execvp();
void Help(char **args)
{
    if (args[1] == NULL)
    {
        char *argv[] = {"cat", "help", NULL};
        execvp(argv[0], argv);
    }
    else if (strcmp(args[1], "cd") == 0)
    {
        char *argv[] = {"cat", "cd", NULL};
        execvp(argv[0], argv);
    }
    else if (strcmp(args[1], "exit") == 0)
    {
        char *argv[] = {"cat", "exit", NULL};
        execvp(argv[0], argv);
    }
}
char **Split(char *cadena, char delimitador[])
{

    char **devuelve = (char **)malloc(512);
    int i = 0;
    char *token = strtok(cadena, delimitador);
    while (token != NULL)
    {
        devuelve[i] = token;
        token = strtok(NULL, delimitador);
        i++;
    }

    return devuelve;
}
char **Parse(char *cadena)
{

    if (cadena[0] == '#')
    {
        return NULL;
    }

    return Split(Split(cadena, "#\n")[0], "|");
}
void Ejecutar(char **args)
{
    if (args[0] == NULL)
    {
        return;
    }
    else if (args[1] == NULL)
    {
        // bool hay_entrada = false;
        // bool hay_salida = false;
        int in_input = -1;
        int in_output = -1;
        char **args_split = Split(args[0], " ");
        for (int i = 0; args_split[i] != NULL; i++)
        {
            if (args_split[i] != NULL && (strcmp(args_split[i], "<") == 0))
            {
                // hay_entrada = true;
                in_input = open(args_split[i + 1], O_RDONLY);
                if (in_input < 0)
                {
                    perror("Failed to open input file");
                    exit(1);
                }
                args_split[i] = NULL;
                // args_split[i + 1] = NULL;
            }
            if (args_split[i] != NULL && (strcmp(args_split[i], ">") == 0 || strcmp(args_split[i], ">>") == 0))
            {
                // hay_salida = true;
                if (strcmp(args_split[i], ">") == 0)
                    in_output = open(args_split[i + 1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
                else
                    in_output = open(args_split[i + 1], O_CREAT | O_WRONLY | O_APPEND, 0644);

                if (in_output < 0)
                {
                    perror("Failed to open output file");
                    exit(1);
                }
                args_split[i] = NULL;
                // args_split[i + 1] = NULL;
            }
        }

        if (args_split[0] == NULL)
        {
            return;
        }
        if (strcmp(args_split[0], "exit") == 0)
        {

            exit(1);
            return;
        }
        else if (strcmp(args_split[0], "cd") == 0)
        {
            if (chdir(args_split[1]) != 0)
            {
                perror(args_split[1]);
            }
            return;
        }
        else if (strcmp(args_split[0], "help") == 0)
        {
            pid_t pid = fork();
            if (pid == -1)
            {
                perror("Error");
                exit(EXIT_FAILURE);
            }
            if (pid == 0)
            {
                Help(args_split);
            }
            waitpid(pid, NULL, 0);
        }

        else
        {

            pid_t pid = fork();
            if (pid == -1)
            {
                perror("Error");
                exit(EXIT_FAILURE);
            }
            if (pid == 0)
            {
                if (in_output != -1)
                {
                    // Output redirection
                    if (dup2(in_output, 1) == -1)
                    {
                        perror("Failed to redirect output");
                        exit(1);
                    }
                }
                if (in_input != -1)
                {
                    // Input redirection
                    if (dup2(in_input, 0) == -1)
                    {
                        perror("Failed to redirect input");
                        exit(1);
                    }
                }

                if (execvp(args_split[0], args_split) == -1)
                {
                    perror("Failed to execute command");
                    exit(1);
                }
            }
            waitpid(pid, NULL, 0);
        }
    }

    else
    {
        char **args_split = Split(args[0], " ");
        if (args_split[0] == NULL)
        {
            return;
        }
        char **args_split1 = Split(args[1], " ");
        if (args_split1[0] == NULL)
        {
            return;
        }
        int pip[2];
        if (pipe(pip) == -1)
        {
            perror("Error-> ");
            exit(EXIT_FAILURE);
        }
        pid_t pid = fork();
        if (pid == -1)
        {
            perror("Error-> ");
        }
        if (pid == 0)
        {
            printf("estoy en el hijo uno\n");
            // hijo
            close(pip[0]);
            dup2(pip[1], 1);
            if (execvp(args_split[0], args_split) == -1)
            {
                perror("Failed to execute command");
                exit(1);
            }
        }
        printf("Se supone que hce algo\n");
        pid_t pid1 = fork();
        if (pid1 == -1)
        {
            perror("Error-> ");
        }
        if (pid1 == 0)
        {
            printf("estoy en mi segundo hijo\n");

            // hijo 2
            close(pip[1]);
            if (dup2(pip[0], 0) == -1)
                perror("Error -> ");
            if (execvp(args_split1[0], args_split1) == -1)
            {
                perror("Failed to execute command");
                exit(1);
            }
            exit(1);
        }
        printf("hice algo en mi hijo 2 que no estoy imprimiendo");
        close(pip[0]);
        close(pip[1]);
        waitpid(pid, NULL, 0);
        waitpid(pid1, NULL, 0);
    }
}

int main()
{
    char entrada[BUFSIZ];
    while (sigue)
    {
        printf("Yoan~Kevin~$ ");
        fgets(entrada, BUFSIZ, stdin);
        char **in_parse = Parse(entrada);
        Ejecutar(in_parse);
    }

    return 0;
}