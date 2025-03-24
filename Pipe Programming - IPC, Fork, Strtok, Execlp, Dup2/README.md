# Pipe Communication Between Processes

## Description

This C program demonstrates pipe communication between two processes using `fork()`, `pipe()`, `execlp()`, and `dup2()` functions. The program takes two commands as arguments, splits them into individual tokens, and creates a pipeline where the output of the first command is fed as input to the second command.


## How It Works
1. **Argument Parsing**: 
   - The program accepts two arguments (commands) passed via the command line.
   - Each command is split into individual tokens.
   
2. **Pipe Creation**: 
   - A pipe is created to allow communication between the two processes.
   
3. **Forking Processes**:
   - The first process writes the output of its command into the pipe.
   - The second process reads the output from the pipe and executes its command with the received input.
   
4. **Executing Commands**: 
   - The program uses `execlp()` to execute the commands in each process.

5. **Parent Process**: 
   - The parent process waits for both child processes to finish execution.

## Compilation and Usage

### Compilation
To compile the program, use the following command:

```bash
gcc -o pipe_program pipe_program.c
```


### Running the Program
You need to provide two commands as arguments. For example, to execute ls followed by grep, run the program as follows:

```bash
./pipe_program "ls -l" "grep '.c'"
```

### Expected Output
If the `ls` command lists `.c` files, the `grep` command will filter them, and the program will output the results of the second command.

### Example
```bash
./pipe_program "ls -l" "grep 'file'"
```
Output:
```bash
-rw-r--r-- 1 user user 1234 Mar 20 14:12 file1.c
-rw-r--r-- 1 user user 4321 Mar 20 14:12 file2.c
```

## Error Handling
- The program checks if the correct number of arguments are provided.
- The program checks if any errors occur during the execution of commands using `execlp()` and handles them using `perror()`.

## Video Peresentation
[Youtube](https://youtu.be/c1SODPgvzO4)

## Licence
 - This program is for educational purposes and is free to use and modify.

## Acknowledgments

- Developed by: Mostafa Hasanalipourshahrabadi
- Tested on a Linux VM.


