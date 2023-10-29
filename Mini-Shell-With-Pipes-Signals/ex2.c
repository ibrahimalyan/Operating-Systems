#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>


#define size_command 510
#define size_argument 10


int num_command = 0;
int num_args = 0;
char current_dir[size_command];
int fg_pid = 0;


void execute_command(char* command);
//this function print the prompt on the screen and inside it count a
//number of command and number of arguments and also the current file by using getcwd
void print_prompt() {
    printf("#cmd:%d|#args:%d@%s> ", num_command, num_args, getcwd(current_dir, sizeof(current_dir)));
}
// this function count how many argument we have by using split the string by strtok
char** count_argument(char* command) {
    char** args = malloc(size_argument * sizeof(char*));
    if (args == NULL ) {
        exit(0);
    }
    char* token = strtok(command, " \n\t");
    int i = 0;
    while (token != NULL && i < size_argument) {
//If the token starts with a $ character, it is assumed to be an environment variable,
        if (token[0] == '$') {
            char* env_var = getenv(&token[1]);
            if (env_var != NULL) {
                args[i++] = env_var;
            }
        } else {
            args[i++] = token;
        }
        token = strtok(NULL, " \n\t");
    }

    args[i] = NULL;
    num_args += i;
    return args;
}
// this function check if we have space
int  space(int c) {
    return (c == ' ' || c == '\t' || c == '\n');
}
//this function removes any leading and trailing whitespace characters.
char* without_space(char* str) {
    char* end = str + strlen(str) - 1;
    //increment the pointer str until it points to the first non-whitespace character.
    while ( space(*str)) {
        str++;
    }
    //decrement the pointer end until it points to the last non-whitespace character.
    while (end > str &&  space(*end)) {
        end--;
    }
    *(end + 1) = '\0';
    //Return the pointer str, which now points to the first non-whitespace character in the string.
    return str;
}
//his function pause a foreground process when the SIGTSTP signal is received
//and provide a message to the user indicating how to resume the process.
void handle_SIGTSTP(int signum) {
    //Check if there is a foreground process currently running
    if (fg_pid > 0) {
        // Send a SIGTSTP signal to the foreground process to pause it
        kill(fg_pid, SIGTSTP);
        printf("\nProcess paused  Type \"bg\" to resume it in the background.\n");
        // is no longer a foreground process running.
        fg_pid = 0;
    } else {
        //If there is no foreground process running, simply print a newline character to the console
        printf("\n");
    }
}
//this function take command to resume a background process with the specified pid
//The cmd string is then passed to another function command1() for execution.
void bg(pid_t pid) {
    char temp[100];
    sprintf(temp, "bg %d", pid);
//Pass the temp string to the execute_command function for execution.
    execute_command(temp);
}

//The function  tokenizes the command by semicolon ; and then executes each individual command sequentially
//The function handles various scenarios such as pipes |, output redirection >, background processes &, and pausing foreground processes using CTRL-Z.
void execute_command(char* command) {
    char *place;
    char *token = strtok_r(command, ";", &place);

    int k = 0;
    signal(SIGTSTP, handle_SIGTSTP);

    while (token != NULL && k <= size_argument) {

        char *pipe_token;
        char *pipe_place;
        char **args;
        int is_pipe = 0;
        int pipefd[2];
        int wait_child = 1;
        int output_redirect = 0;
        char *output_file = NULL;

//checks for & at the end of the token to determine whether to wait for the child process to complete or not.
        if (strstr(token, "&") != NULL) {
            wait_child = 0
                    //token string by replacing the last character
                    //By doing this, the command is modified to remove the ampersand
                    // as it's not part of the actual command and is used only as a directive to run the command in the background.
            token[strlen(token) - 1] = '\0';
        }

//The code snippet checks for output redirection (using ">") in the given token string. It extracts the output token and output file name (if present),
        if (strstr(token, ">") != NULL) {
            output_redirect = 1;
            char *output_token = strtok_r(token, ">", &pipe_place);
            args = count_argument(output_token);
            output_file = strtok_r(NULL, ">", &pipe_place);
            if (output_file != NULL) {
//and applies some processing to remove spaces from the output file name
                output_file = without_space(output_file);
            }
        } else if (strstr(token, "|") != NULL) {
            //This extracts the first part of the piped command
            pipe_token = strtok_r(token, "|", &pipe_place);
            //: The first part of the piped command is counted and stored in the args array, ready for execution.
            args = count_argument(pipe_token);
            if (pipe(pipefd) == -1) {
                perror("ERR");
                exit(0);
            }
// sets a flag (is_pipe) to indicate the presence of a pipe
            is_pipe = 1;
        } else {
            args = count_argument(token);
        }

        if (args[0] != NULL) {
            num_args += (int) (sizeof(args) / sizeof(char *) - 1);

            if (strcmp(args[0], "cd") == 0) {
                printf("cd not supported\n");
            }
            else if (strcmp(args[0], "bg") == 0) {
                if (fg_pid > 0) {
                    bg(fg_pid);
                } else {
                    printf(" process resume in the background.\n");
                }
            }

            else {

                pid_t pid = fork();
                if (pid < 0) {
                    perror("ERR");
                    exit(0);
                } else if (pid == 0) {
                    if (output_redirect) {
                        //write the file
                        FILE *file = freopen(output_file, "w", stdout);
                        if (file == NULL) {
                            perror("ERR");
                            exit(0);
                        }
                    } else if (is_pipe) {
                        dup2(pipefd[1], STDOUT_FILENO);
                        close(pipefd[0]);
                    }
                    execvp(args[0], args);
                    perror("ERR");
                    exit(0);
                } else {
                    if (is_pipe) {
                        pid_t pid2 = fork();
                        if (pid2 < 0) {
                            perror("ERR");
                            exit(0);
                        } else if (pid2 == 0) {
                            //read
                            dup2(pipefd[0], STDIN_FILENO);
                            //close the write
                            close(pipefd[1]);
                            //This extracts the second part of the piped command
                            char *pipe_token2 = strtok_r(NULL, "|", &pipe_place);
                            //: The second part of the piped command is counted and stored in the args2 array, ready for execution.
                            char **args2 = count_argument(pipe_token2);
                            execvp(args2[0], args2);
                            perror("ERR");
                            exit(0);
                        } else {
                            close(pipefd[0]);
                            close(pipefd[1]);
                            if (wait_child) {
                                waitpid(pid2, NULL, 0);
                            }
                        }
                    } else {
                        // Set the foreground process ID
                        fg_pid = pid;
                        if (wait_child) {
                            // Wait for the foreground process to finish or get paused
                            waitpid(pid, NULL, WUNTRACED);
                            // Reset the foreground process ID
                            fg_pid = 0;
                        }
                    }
                }
            }



            free(args);
            token = strtok_r(NULL, ";", &place);
            num_command++;
        }
    }}
//function then extracts the environment variable name and value
// from the command string and sets the environment variable using the setenv() function.
void  extracts_env(char* command) {
    int i = 0;
    char* token = strtok(command, "=");
    char* env_name = token;
    while (token != NULL) {
        i++;
//Check if i is equal to 2. If it is, then the current token is the environment variable value
        if (i == 2) {
            char* env_value = token;
//Store the environment variable name in the env_name variable.
            setenv(env_name, env_value, 1);
            break;
        }
        token = strtok(NULL, "=");
    }
}

int main() {
    int ENTER = 0;
    char command[size_command];

    while (1) {
        print_prompt();

        if (fgets(command, size_command, stdin) == NULL) {
            break;
        }

        if (strcmp(command, "\n") == 0) {
            ENTER++;
            if (ENTER == 3) {
                break;
            }
            continue;
        }

        ENTER = 0;

        if (command[strlen(command) - 1] == '\n') {
            command[strlen(command) - 1] = '\0';
        }

        if (strstr(command, "=") != NULL) {
             extracts_env(command);
        } else {
            execute_command(command);
        }
    }

    return 0;
}