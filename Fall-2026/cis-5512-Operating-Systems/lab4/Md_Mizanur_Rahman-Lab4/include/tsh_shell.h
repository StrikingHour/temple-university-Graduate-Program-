#ifndef TSH_SHELL_H
#define TSH_SHELL_H
#include <stdint.h>
#define TSH_OP_SHELL 417
#define TSH_OP_KILL 418
#define TSH_OP_PS 419
#define TSH_OP_KILLALL 420
#define TSH_COMMAND_SIZE 1024
/* All integer fields use network byte order. */
typedef struct {
    uint32_t background;
    char command[TSH_COMMAND_SIZE];
} tsh_shell_request;
typedef struct {
    uint32_t accepted;
    uint32_t pid;
    uint32_t completed;
    uint32_t exit_code;
    uint32_t error_number;
} tsh_shell_response;
typedef struct {
    uint32_t pid;
} tsh_kill_request;
typedef struct {
    uint32_t success;
    uint32_t count;
    uint32_t error_number;
} tsh_control_response;
typedef struct {
    uint32_t pid;
    uint32_t background;
    uint32_t stopping;
    char command[TSH_COMMAND_SIZE];
} tsh_ps_entry;
void shell_service_ps(int fd);
void shell_service_kill(int fd);
void shell_service_killall(int fd);
int shell_service_init(void);
int shell_service_launch(int fd); /* 1: service owns fd; 0: caller closes */
void shell_service_reap(void);
void shell_service_shutdown(void);
#endif
