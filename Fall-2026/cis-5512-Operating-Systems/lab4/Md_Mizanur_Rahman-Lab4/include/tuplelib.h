#ifndef TUPLELIB_H
#define TUPLELIB_H
#include <stddef.h>
/* Same-host toy daemon: TSHHOST must be 127.0.0.1 or localhost.
   Returns 0 on success, -1 with errno on failure. take=0 READ, take=1 GET.
   Fetch allocates *data; caller frees it. Patterns are POSIX basic regex. */
int tuple_put(const char *name, const void *data, size_t bytes);
int tuple_fetch(const char *pattern, int take, void **data, size_t *bytes);
#endif
