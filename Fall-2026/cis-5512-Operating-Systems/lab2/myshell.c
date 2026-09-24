#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>

#define MSH_RL_BUFSIZE 1024
#define MSH_TOK_BUFSIZE 64
#define MSH_TOK_DELIM " \t\r\n\a"


/* =========================================================
 * FUNCTION PROTOTYPES
 * =========================================================
 */

char *msh_read_line(void);
char **msh_split_line(char *line);

int msh_cd(char **args);
int msh_help(char **args);
int msh_exit(char **args);

void msh_wait_for_child(pid_t pid);

int msh_apply_output_redirection(char **args);

int msh_launch(char **args);

int msh_count_pipes(char **args);
int msh_execute_pipeline(char **args);
int msh_execute_builtin_child(char **args);

int msh_execute(char **args);

void msh_loop(void);


/* =========================================================
 * READ COMMAND LINE
 * =========================================================
 */

char *msh_read_line(void)
{
    int bufsize = MSH_RL_BUFSIZE;
    int position = 0;

    char *buffer =
        malloc(sizeof(char) * bufsize);

    if (buffer == NULL) {
        perror("msh: malloc");
        exit(EXIT_FAILURE);
    }

    while (1) {

        int c = getchar();

        /*
         * Ctrl+D / EOF
         */
        if (c == EOF) {

            if (position == 0) {
                free(buffer);
                return NULL;
            }

            buffer[position] = '\0';
            return buffer;
        }

        /*
         * Enter key
         */
        if (c == '\n') {
            buffer[position] = '\0';
            return buffer;
        }

        buffer[position] = (char)c;
        position++;

        /*
         * Increase buffer size if necessary.
         */
        if (position >= bufsize) {

            bufsize += MSH_RL_BUFSIZE;

            char *new_buffer =
                realloc(
                    buffer,
                    sizeof(char) * bufsize
                );

            if (new_buffer == NULL) {
                free(buffer);
                perror("msh: realloc");
                exit(EXIT_FAILURE);
            }

            buffer = new_buffer;
        }
    }
}


/* =========================================================
 * SPLIT COMMAND LINE INTO TOKENS
 * =========================================================
 */

char **msh_split_line(char *line)
{
    int bufsize = MSH_TOK_BUFSIZE;
    int position = 0;

    char **tokens =
        malloc(sizeof(char *) * bufsize);

    if (tokens == NULL) {
        perror("msh: malloc");
        exit(EXIT_FAILURE);
    }

    char *token =
        strtok(
            line,
            MSH_TOK_DELIM
        );

    while (token != NULL) {

        tokens[position] = token;
        position++;

        /*
         * Increase token array if needed.
         */
        if (position >= bufsize) {

            bufsize += MSH_TOK_BUFSIZE;

            char **new_tokens =
                realloc(
                    tokens,
                    sizeof(char *) * bufsize
                );

            if (new_tokens == NULL) {
                free(tokens);
                perror("msh: realloc");
                exit(EXIT_FAILURE);
            }

            tokens = new_tokens;
        }

        token =
            strtok(
                NULL,
                MSH_TOK_DELIM
            );
    }

    /*
     * execvp() requires argv to end with NULL.
     */
    tokens[position] = NULL;

    return tokens;
}


/* =========================================================
 * BUILT-IN COMMAND: cd
 * =========================================================
 */

int msh_cd(char **args)
{
    if (args[1] == NULL) {

        fprintf(
            stderr,
            "msh: expected argument to \"cd\"\n"
        );

        return 1;
    }

    if (chdir(args[1]) == -1) {
        perror("msh: cd");
    }

    return 1;
}


/* =========================================================
 * BUILT-IN COMMAND: help
 * =========================================================
 */

int msh_help(char **args)
{
    (void)args;

    printf("Simple Unix Shell\n\n");

    printf("Built-in commands:\n");
    printf("  cd <directory>   Change directory\n");
    printf("  help             Display this help message\n");
    printf("  exit             Exit the shell\n");

    printf("\nSupported features:\n");
    printf("  Normal Unix commands\n");
    printf("  Single pipes\n");
    printf("  Multiple pipes\n");
    printf("  >  output redirection\n");
    printf("  >> append redirection\n");

    printf("\nExamples:\n");
    printf("  ls -l\n");
    printf("  ls | wc\n");
    printf("  ls -l | grep myshell | wc\n");
    printf("  echo hello > file.txt\n");
    printf("  echo hello >> file.txt\n");
    printf("  ls -l | wc > count.txt\n");

    return 1;
}


/* =========================================================
 * BUILT-IN COMMAND: exit
 * =========================================================
 */

int msh_exit(char **args)
{
    (void)args;

    return 0;
}

/*
 * Execute a built-in command inside a pipeline child.
 *
 * Returns:
 *   1 = command was a built-in and was executed
 *   0 = command was not a built-in
 */
int msh_execute_builtin_child(char **args)
{
    if (args == NULL || args[0] == NULL) {
        return 0;
    }

    /*
     * cd inside a pipeline changes only
     * this child process's directory.
     */
    if (strcmp(args[0], "cd") == 0) {

        msh_cd(args);

        return 1;
    }

    /*
     * help can write its output into a pipe.
     */
    if (strcmp(args[0], "help") == 0) {

        msh_help(args);

        /*
         * IMPORTANT:
         *
         * stdout may be connected to a pipe.
         * Because we later use _exit(), stdio
         * buffers would NOT automatically flush.
         */
        if (fflush(stdout) == EOF) {
            perror("msh: fflush");
        }

        return 1;
    }

    /*
     * exit inside a pipeline exits only
     * this pipeline child, not the parent shell.
     */
    if (strcmp(args[0], "exit") == 0) {

        msh_exit(args);

        return 1;
    }

    return 0;
}


/* =========================================================
 * WAIT FOR CHILD PROCESS
 * =========================================================
 */

void msh_wait_for_child(pid_t pid)
{
    int status;

    while (1) {

        pid_t result =
            waitpid(
                pid,
                &status,
                0
            );

        if (result == -1) {

            /*
             * If interrupted by signal,
             * try waitpid again.
             */
            if (errno == EINTR) {
                continue;
            }

            perror("msh: waitpid");
            return;
        }

        if (
            WIFEXITED(status) ||
            WIFSIGNALED(status)
        ) {

            return;
        }
    }
}


/* =========================================================
 * OUTPUT REDIRECTION
 *
 * Supports:
 *
 * command > file
 * command >> file
 * =========================================================
 */

int msh_apply_output_redirection(char **args)
{
    int redirect_index = -1;
    int append_mode = 0;

    /*
     * Search for > or >>.
     */
    for (int i = 0;
         args[i] != NULL;
         i++) {

        if (
            strcmp(args[i], ">") == 0 ||
            strcmp(args[i], ">>") == 0
        ) {

            /*
             * Only one output redirection
             * is supported.
             */
            if (redirect_index != -1) {

                fprintf(
                    stderr,
                    "msh: multiple output redirections "
                    "are not supported\n"
                );

                return -1;
            }

            redirect_index = i;

            /*
             * Determine whether this is >>
             */
            if (strcmp(args[i], ">>") == 0) {
                append_mode = 1;
            }
        }
    }

    /*
     * No output redirection.
     */
    if (redirect_index == -1) {
        return 0;
    }

    /*
     * Invalid:
     *
     * > file.txt
     */
    if (redirect_index == 0) {

        fprintf(
            stderr,
            "msh: missing command before redirection\n"
        );

        return -1;
    }

    /*
     * Invalid:
     *
     * ls >
     */
    if (
        args[
            redirect_index + 1
        ] == NULL
    ) {

        fprintf(
            stderr,
            "msh: expected filename after redirection\n"
        );

        return -1;
    }

    /*
     * Require filename to be last argument.
     *
     * Valid:
     *
     * ls > output.txt
     *
     * Invalid:
     *
     * ls > output.txt extra
     */
    if (
        args[
            redirect_index + 2
        ] != NULL
    ) {

        fprintf(
            stderr,
            "msh: output filename must be "
            "the last argument\n"
        );

        return -1;
    }

    char *filename =
        args[
            redirect_index + 1
        ];

    /*
     * Remove redirection syntax from argv.
     *
     * Before:
     *
     * ["wc", ">", "count.txt", NULL]
     *
     * After:
     *
     * ["wc", NULL]
     */
    args[redirect_index] = NULL;

    /*
     * Common flags:
     *
     * O_WRONLY = write only
     * O_CREAT  = create file if necessary
     */
    int flags =
        O_WRONLY | O_CREAT;

    /*
     * >>
     */
    if (append_mode) {

        flags |= O_APPEND;
    }

    /*
     * >
     */
    else {

        flags |= O_TRUNC;
    }

    /*
     * Open file.
     *
     * 0644 permissions:
     *
     * owner  = read/write
     * group  = read
     * others = read
     */
    int fd =
        open(
            filename,
            flags,
            0644
        );

    if (fd == -1) {

        perror("msh: open");

        return -1;
    }

    /*
     * Redirect stdout to file.
     */
    if (
        dup2(
            fd,
            STDOUT_FILENO
        ) == -1
    ) {

        perror("msh: dup2");

        if (fd != STDOUT_FILENO) {
            close(fd);
        }

        return -1;
    }

    /*
     * Close original file descriptor.
     */
    if (fd != STDOUT_FILENO) {

        if (close(fd) == -1) {

            perror("msh: close");

            return -1;
        }
    }

    return 1;
}


/* =========================================================
 * EXECUTE NORMAL EXTERNAL COMMAND
 * =========================================================
 */

int msh_launch(char **args)
{
    pid_t pid =
        fork();

    /*
     * fork() failed.
     */
    if (pid < 0) {

        perror("msh: fork");

        return 1;
    }

    /*
     * CHILD
     */
    if (pid == 0) {

        /*
         * Apply > or >> before execvp().
         */
        if (
            msh_apply_output_redirection(
                args
            ) == -1
        ) {

            _exit(EXIT_FAILURE);
        }

        execvp(
            args[0],
            args
        );

        /*
         * execvp only returns if it fails.
         */
        perror("msh: execvp");

        _exit(EXIT_FAILURE);
    }

    /*
     * PARENT
     */
    msh_wait_for_child(pid);

    return 1;
}


/* =========================================================
 * COUNT PIPE SYMBOLS
 * =========================================================
 */

int msh_count_pipes(char **args)
{
    int count = 0;

    for (int i = 0;
         args[i] != NULL;
         i++) {

        if (
            strcmp(
                args[i],
                "|"
            ) == 0
        ) {

            count++;
        }
    }

    return count;
}


/* =========================================================
 * EXECUTE PIPELINE
 *
 * Supports:
 *
 * ls | wc
 *
 * ls -l | grep myshell | wc
 *
 * ls -l | wc > count.txt
 *
 * ls -l | grep myshell | wc > result.txt
 * =========================================================
 */

int msh_execute_pipeline(char **args)
{
    int argc = 0;
    int command_count = 1;

    /*
     * Count total tokens.
     */
    while (
        args[argc] != NULL
    ) {

        argc++;
    }

    if (argc == 0) {
        return 1;
    }

    /*
     * Invalid:
     *
     * | wc
     */
    if (
        strcmp(
            args[0],
            "|"
        ) == 0
    ) {

        fprintf(
            stderr,
            "msh: invalid pipe syntax\n"
        );

        return 1;
    }

    /*
     * Invalid:
     *
     * ls |
     */
    if (
        strcmp(
            args[argc - 1],
            "|"
        ) == 0
    ) {

        fprintf(
            stderr,
            "msh: invalid pipe syntax\n"
        );

        return 1;
    }

    /*
     * Count commands and check for:
     *
     * ls | | wc
     */
    for (int i = 0;
         i < argc;
         i++) {

        if (
            strcmp(
                args[i],
                "|"
            ) == 0
        ) {

            if (
                i > 0 &&
                strcmp(
                    args[i - 1],
                    "|"
                ) == 0
            ) {

                fprintf(
                    stderr,
                    "msh: invalid pipe syntax\n"
                );

                return 1;
            }

            command_count++;
        }
    }

    /*
     * Array containing pointers to
     * individual commands.
     */
    char ***commands =
        malloc(
            command_count *
            sizeof(char **)
        );

    if (commands == NULL) {

        perror("msh: malloc");

        return 1;
    }

    /*
     * First command begins at args[0].
     */
    commands[0] = args;

    int command_index = 1;

    /*
     * Split argv at each |.
     *
     * Example:
     *
     * ls -l | grep msh | wc
     *
     * becomes:
     *
     * ["ls", "-l", NULL]
     * ["grep", "msh", NULL]
     * ["wc", NULL]
     */
    for (int i = 0;
         i < argc;
         i++) {

        if (
            strcmp(
                args[i],
                "|"
            ) == 0
        ) {

            /*
             * End previous command.
             */
            args[i] = NULL;

            /*
             * Next command starts after |.
             */
            commands[
                command_index
            ] =
                &args[
                    i + 1
                ];

            command_index++;
        }
    }

    /*
     * Store child PIDs.
     */
    pid_t *pids =
        malloc(
            command_count *
            sizeof(pid_t)
        );

    if (pids == NULL) {

        perror("msh: malloc");

        free(commands);

        return 1;
    }

    /*
     * Read side of previous pipe.
     *
     * -1 means there is no previous pipe.
     */
    int prev_read = -1;

    int children_started = 0;


    /* =====================================================
     * CREATE PIPELINE CHILDREN
     * =====================================================
     */

    for (int i = 0;
         i < command_count;
         i++) {

        int next_pipe[2] =
            {-1, -1};

        /*
         * Every command except the last
         * needs an output pipe.
         */
        if (
            i <
            command_count - 1
        ) {

            if (
                pipe(
                    next_pipe
                ) == -1
            ) {

                perror("msh: pipe");

                if (prev_read != -1) {

                    if (
                        close(
                            prev_read
                        ) == -1
                    ) {

                        perror(
                            "msh: close"
                        );
                    }
                }

                /*
                 * Wait for children already created.
                 */
                for (
                    int j = 0;
                    j < children_started;
                    j++
                ) {

                    msh_wait_for_child(
                        pids[j]
                    );
                }

                free(pids);
                free(commands);

                return 1;
            }
        }

        /*
         * Create child.
         */
        pid_t pid =
            fork();

        /*
         * fork() failed.
         */
        if (pid < 0) {

            perror("msh: fork");

            /*
             * Close newly-created pipe.
             */
            if (
                i <
                command_count - 1
            ) {

                if (
                    close(
                        next_pipe[0]
                    ) == -1
                ) {

                    perror(
                        "msh: close"
                    );
                }

                if (
                    close(
                        next_pipe[1]
                    ) == -1
                ) {

                    perror(
                        "msh: close"
                    );
                }
            }

            /*
             * Close previous pipe read end.
             */
            if (prev_read != -1) {

                if (
                    close(
                        prev_read
                    ) == -1
                ) {

                    perror(
                        "msh: close"
                    );
                }
            }

            /*
             * Wait for previously created children.
             */
            for (
                int j = 0;
                j < children_started;
                j++
            ) {

                msh_wait_for_child(
                    pids[j]
                );
            }

            free(pids);
            free(commands);

            return 1;
        }


        /* =================================================
         * CHILD PROCESS
         * =================================================
         */

        if (pid == 0) {

            /*
             * If this is not the first command,
             * stdin comes from previous pipe.
             */
            if (prev_read != -1) {

                if (
                    dup2(
                        prev_read,
                        STDIN_FILENO
                    ) == -1
                ) {

                    perror(
                        "msh: dup2 stdin"
                    );

                    _exit(
                        EXIT_FAILURE
                    );
                }
            }

            /*
             * If this is not the last command,
             * stdout goes to next pipe.
             */
            if (
                i <
                command_count - 1
            ) {

                if (
                    dup2(
                        next_pipe[1],
                        STDOUT_FILENO
                    ) == -1
                ) {

                    perror(
                        "msh: dup2 stdout"
                    );

                    _exit(
                        EXIT_FAILURE
                    );
                }
            }

            /*
             * Close original previous pipe descriptor.
             */
            if (
                prev_read != -1 &&
                prev_read != STDIN_FILENO
            ) {

                if (
                    close(
                        prev_read
                    ) == -1
                ) {

                    perror(
                        "msh: close"
                    );

                    _exit(
                        EXIT_FAILURE
                    );
                }
            }

            /*
             * Close original descriptors
             * of next pipe.
             */
            if (
                i <
                command_count - 1
            ) {

                if (
                    next_pipe[0] !=
                    STDIN_FILENO &&
                    next_pipe[0] !=
                    STDOUT_FILENO
                ) {

                    if (
                        close(
                            next_pipe[0]
                        ) == -1
                    ) {

                        perror(
                            "msh: close"
                        );

                        _exit(
                            EXIT_FAILURE
                        );
                    }
                }

                if (
                    next_pipe[1] !=
                    STDOUT_FILENO
                ) {

                    if (
                        close(
                            next_pipe[1]
                        ) == -1
                    ) {

                        perror(
                            "msh: close"
                        );

                        _exit(
                            EXIT_FAILURE
                        );
                    }
                }
            }

            /*
             * IMPORTANT:
             *
             * Apply > or >> AFTER pipe setup
             * but BEFORE execvp().
             *
             * Example:
             *
             * ls -l | wc > count.txt
             *
             * For wc:
             *
             * ["wc", ">", "count.txt", NULL]
             *
             * becomes:
             *
             * ["wc", NULL]
             *
             * and stdout becomes count.txt.
             */
            if (
                msh_apply_output_redirection(
                    commands[i]
                ) == -1
            ) {

                _exit(
                    EXIT_FAILURE
                );
            }
			
			/*
			* --------------------------------
			* CHECK BUILT-IN COMMAND
			* --------------------------------
			*
			* Built-ins in a pipeline must run
			* inside this child process.
			*/
			if(msh_execute_builtin_child(commands[i])){

				/*
				* The built-in has finished.
				*
				* Terminate this CHILD only.
				*/
				_exit(EXIT_SUCCESS);
			}
			
			

            /*
             * Execute command.
             */
            execvp(
                commands[i][0],
                commands[i]
            );

            /*
			* execvp() only returns on failure.
			*/
			fprintf(stderr,"msh: %s: %s\n",commands[i][0],strerror(errno));

            _exit(
                EXIT_FAILURE
            );
        }


        /* =================================================
         * PARENT PROCESS
         * =================================================
         */

        pids[
            children_started
        ] = pid;

        children_started++;

        /*
         * Parent no longer needs the
         * previous pipe read end.
         */
        if (prev_read != -1) {

            if (
                close(
                    prev_read
                ) == -1
            ) {

                perror(
                    "msh: close"
                );
            }

            prev_read = -1;
        }

        /*
         * Parent closes write side of
         * next pipe and keeps read side
         * for the next child.
         */
        if (
            i <
            command_count - 1
        ) {

            if (
                close(
                    next_pipe[1]
                ) == -1
            ) {

                perror(
                    "msh: close"
                );
            }

            prev_read =
                next_pipe[0];
        }
    }


    /* =====================================================
     * WAIT FOR ALL PIPELINE CHILDREN
     * =====================================================
     */

    for (
        int i = 0;
        i < children_started;
        i++
    ) {

        msh_wait_for_child(
            pids[i]
        );
    }

    free(pids);
    free(commands);

    return 1;
}


/* =========================================================
 * DETERMINE HOW COMMAND SHOULD EXECUTE
 * =========================================================
 */

int msh_execute(char **args)
{
    /*
     * Empty command.
     */
    if (
        args == NULL ||
        args[0] == NULL
    ) {

        return 1;
    }

    /*
     * Pipeline command.
     */
    if (
        msh_count_pipes(args) > 0
    ) {

        return
            msh_execute_pipeline(
                args
            );
    }

    /*
     * Built-in cd.
     */
    if (
        strcmp(
            args[0],
            "cd"
        ) == 0
    ) {

        return
            msh_cd(args);
    }

    /*
     * Built-in help.
     */
    if (
        strcmp(
            args[0],
            "help"
        ) == 0
    ) {

        return
            msh_help(args);
    }

    /*
     * Built-in exit.
     */
    if (
        strcmp(
            args[0],
            "exit"
        ) == 0
    ) {

        return
            msh_exit(args);
    }

    /*
     * Normal Unix command.
     */
    return
        msh_launch(args);
}


/* =========================================================
 * MAIN SHELL LOOP
 * =========================================================
 */

void msh_loop(void)
{
    int status = 1;

    while (status) {

        /*
         * Display prompt.
         */
        printf("msh> ");
        fflush(stdout);

        /*
         * Read command.
         */
        char *line =
            msh_read_line();

        /*
         * Ctrl+D
         */
        if (line == NULL) {

            printf("\n");

            break;
        }

        /*
         * Split line into arguments.
         */
        char **args =
            msh_split_line(
                line
            );

        /*
         * Execute command.
         */
        status =
            msh_execute(
                args
            );

        /*
         * Tokens point inside line,
         * so only free args and line.
         */
        free(args);
        free(line);
    }
}


/* =========================================================
 * MAIN
 * =========================================================
 */

int main(void)
{
    msh_loop();

    return EXIT_SUCCESS;
}