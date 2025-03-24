#include <stdio.h>      
#include <stdlib.h>     // For exit()
#include <unistd.h>     // For fork(), pipe(), execlp(), dup2(), close(), waitpid()
#include <sys/types.h>  // For pid_t 
#include <sys/wait.h>   // For waitpid()
#include <string.h>     // For strcpy(), strtok()
#include <errno.h>      // For perror()

#define LEN 32


int main(int argc, char* argv[]) {

    if (argc != 3) {
        perror("Not enough arguments are provided.");
        exit(1);
    }

    int pipefd[2]; 
    char argument1[LEN];
    char argument2[LEN];

    strcpy(argument1, argv[1]);
    strcpy(argument2, argv[2]);

    char arg1[3][LEN];
    char arg2[3][LEN];
    int len1 = 0, len2 = 0;

    // Tokenize argument1
    char* token = strtok(argument1, " ");
    while (token != NULL) {
        strcpy(arg1[len1], token);
        token = strtok(NULL, " ");
        ++len1;
    }

    // Tokenize argument2
    token = strtok(argument2, " ");
    while (token != NULL) {
        strcpy(arg2[len2], token);
        token = strtok(NULL, " ");
        ++len2;
    }

    char* arr1[len1 + 1];
    char* arr2[len2 + 1];

    for (int i = 0; i < len1; ++i) {
        arr1[i] = arg1[i];
    }
    arr1[len1] = NULL;

    for (int i = 0; i < len2; ++i) {
        arr2[i] = arg2[i];
    }
    arr2[len2] = NULL;

    // Create the pipe
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(-1);
    }

    pid_t pid1 = fork();

    // Child 1, write end of pipe
    if (pid1 == 0) {

        // Close read end of pipe
        close(pipefd[0]);
        // Redirect stdout to pipe write end
        dup2(pipefd[1], STDOUT_FILENO);

        // Close write end of pipe
        close(pipefd[1]);

        // Using execlp to execute command1 with individual arguments
        if (len1 == 1) {
            execlp(arr1[0], arr1[0], (char*)NULL);
        }
        else if (len1 == 2) { // ls -l
            execlp(arr1[0], arr1[0], arr1[1], (char*)NULL);
        }
        else if (len1 == 3) { // ls -l -a
            execlp(arr1[0], arr1[0], arr1[1], arr1[2], (char*)NULL);
        }

        // If execlp fails
        perror("execlp failed");  
        exit(1);
    }

    // Child 2, Read end of pipe
    pid_t pid2 = fork();
    if (pid2 == 0) {
        // Close write end of pipe
        close(pipefd[1]);
        // Redirect stdin to pipe read end
        dup2(pipefd[0], STDIN_FILENO);

        // Close read end of pipe
        close(pipefd[0]);

        // Using execlp to execute command2 with individual arguments
        if (len2 == 1) {
            execlp(arr2[0], arr2[0], (char*)NULL);
        }
        else if (len2 == 2) { 
            execlp(arr2[0], arr2[0], arr2[1], (char*)NULL);
        }
        else if (len2 == 3) { 
            execlp(arr2[0], arr2[0], arr2[1], arr2[2], (char*)NULL);
        }

        // If execlp fails
        perror("execlp");
        exit(1);
    }

    // Parent process closes both pipe ends
    close(pipefd[0]);
    close(pipefd[1]);

    // Wait for both child processes
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    return 0;
}




