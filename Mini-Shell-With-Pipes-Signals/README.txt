Mni shell
Authored by Ibrahim Alyan
322675356
---Description---
This program we will implement a mini shell in the C language by Linux  The shell will display a prompt to the user, read the commands and send them to the operating system for execution in this exercise added some elements like a pipe and & and > and Ctrl z , bg 

---functions---
1- print_prompt: this function print the prompt on the screen and inside it count a number of command and number of arguments and also the current file by using getcwd

2-count_argument: this function count how many argument we have by using split the string by strtok

3-execute_command: The function  tokenizes the command by semicolon ; and then executes each individual command sequentially. The function handles various scenarios such as pipes |, output redirection >, background processes &, and pausing foreground processes using CTRL-Z.

4- space : this function check if we have space 

5-without_space: this function removes any leading and trailing whitespace characters.

6- handle_SIGTSTP: this function pause a foreground process when the SIGTSTP signal is received, and provide a message to the user indicating how to resume the process.

7-bg: this function take command to resume a background process with the specified pid. The cmd string is then passed to another function command1() for execution.

8-extracts_env: function then extracts the environment variable name and value from the command string and sets the environment variable using the setenv() function.

---Program fiels---
ex2.c    this file contain the function and the main

---How to compile---
compile: gcc ex2.c -o ex2
run ; ./ex2

---input---
sleep 10 &

---output---
Go to the background 

---input---
ls > aa
cat aa

---output---
Creat file aa and override ls inside this file aa


---input---
sleep 10 
During the the 10 seconds press Ctrl z 
bg

---output---
pausing foreground processes using CTRL-Z.  and when press bg continue in  background 

---input---
ls -l | wc -l 

---output---
Print now many lines in ls -l


 