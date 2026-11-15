#ifndef TSHLIB_H
#define TSHLIB_H
#include <stddef.h>
#include <stdint.h>
#include "tsh_shell.h"
/* Caller-facing structures contain HOST-order integers.
 * Return 0 on successful request, -1 on transport/server error (errno set).
 * A nonzero command exit status is a completed command, not an API error.
 * Foreground shell calls intentionally block until completion.
 */
typedef struct {
    uint32_t pid, completed, exit_code;
} tsh_job_result;
typedef struct {
    uint32_t pid, background, stopping;
    char command[TSH_COMMAND_SIZE];
} tsh_job_info;
int tsh_shell(const char *host, uint16_t port, const char *command,
              int background, tsh_job_result *result);
int tsh_ps(const char *host, uint16_t port, tsh_job_info *jobs,
           size_t capacity, size_t *count);
int tsh_kill(const char *host, uint16_t port, uint32_t pid);
/* On partial server failure, count still reports successfully signalled jobs. */
int tsh_killall(const char *host, uint16_t port, uint32_t *count);
#endif
