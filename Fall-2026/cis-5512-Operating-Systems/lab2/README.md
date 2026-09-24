# CIS 5512 Lab 3 - Unix Shell

## Student Information

Name: Md. Mizanur Rahman
Course: CIS 5512 Systems Programming

## 1. Project Objective

The objective of this lab is to implement a Unix shell
in C capable of executing Unix commands, supporting
single and multiple pipes, and supporting > and >>
output redirection.

## 2. Features Implemented

- Normal Unix command execution
- Command arguments
- cd built-in
- help built-in
- exit built-in
- Single pipe
- Multiple pipes
- > output redirection
- >> append redirection
- Pipe with output redirection
- Error handling

## 3. Program Design

### 3.1 Input and Parsing

Explain how input is read and split into tokens.

### 3.2 Process Creation

Explain fork(), execvp(), and waitpid().

### 3.3 Built-in Commands

Explain cd, help, and exit.

### 3.4 Pipes

Explain pipe(), dup2(), pipefd[0], and pipefd[1].

### 3.5 Multiple Pipes

Explain that N commands require N-1 pipes.

### 3.6 Redirection

Explain > and >>.

## 4. Compilation

gcc -Wall -Wextra -std=c11 -g myshell.c -o myshell

## 5. Running

./myshell

## 6. AI-Assisted Development

The AI conversation used during development is documented
in ai_log.md.

## 7. Debugging

Summarize the main bugs you found and fixed.

## 8. Test Cases

See test_results.txt.

## 9. Test Results

All required functionality was tested against expected
Unix/Bash behavior.

## 10. Known Limitations

[List only real limitations.]

## 11. Screenshots

01_compile.png - successful compilation
02_basic_commands.png - basic Unix commands
03_builtins.png - built-in commands
04_single_pipe.png - single pipe
05_multiple_pipes.png - multiple pipes
06_redirection_overwrite.png - > redirection
07_redirection_append.png - >> redirection
08_pipe_redirection.png - pipe + redirection
09_error_handling.png - invalid input/error behavior