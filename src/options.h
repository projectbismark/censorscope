#ifndef CENSORSCOPE_OPTIONS_H
#define CENSORSCOPE_OPTIONS_H

#include <stddef.h>

#include "lua.h"

typedef struct {
    char *sandbox_dir;
    char *luasrc_dir;
    char *results_dir;
    /* The total amount of memory available to the interpreter for evaluating
     * the environment and running the sandboxed code. */
    size_t max_memory;
    /* The total number of instructions available to evaluate both the
     * environment and the script itself. Note that is the count of Lua
     * interpreter instructions, not CPU instructions. If a script can call out
     * to C functions, this instructions limitation will not gauarantee that the
     * script completes within a fixed amount of time, especially if sleeping or
     * waiting on I/O is possible. In that case you should enforce operating
     * system resource limits. */
    long max_instructions;
    char *download_transport;
    char *upload_transport;
} censorscope_options_t;

int censorscope_options_init(censorscope_options_t *options,
                             int argc,
                             char **argv);

int censorscope_options_lua(const censorscope_options_t *options, lua_State *L);

int censorscope_options_destroy(censorscope_options_t *options);

#endif
