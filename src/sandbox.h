#ifndef _CENSORSCOPE_SANDBOX_H_
#define _CENSORSCOPE_SANDBOX_H_

#include "lua.h"

typedef struct {
    lua_State *L;
    int available_memory;
} sandbox_t;

/* Initialize a sandbox, which you can use to run Lua code with memory and
 * instruction count constraints.
 *
 * Arguments:
 * - name is a name for the sandbox, which will be exposed when evaluating the
 *   environment.
 * - max_memory is the total amount of memory available to the interpreter for
 *   evaluating the environment and running the sandboxed code.
 * - max_instructions is the total number of instructions available to evaluate
 *   both the environment and the script itself. Note that is the count of Lua
 *   interpreter instructions, not CPU instructions. If a script can call out to
 *   C functions, this instructions limitation will not gauarantee that the
 *   script completes within a fixed amount of time, especially if sleeping or
 *   waiting on I/O is possible. In that case you should enforce operating
 *   system resource limits.
 * Returns: 0 on success, -1 on failure.
 *
 */
int sandbox_init(sandbox_t *sandbox,
                 const char *name,
                 int max_memory,
                 int max_instructions);

int sandbox_destroy(sandbox_t *sandbox);

/* Run a file inside a sandbox with memory and instruction count constraints.
 *
 * Arguments:
 * - filename is the path of the script to run in the sandbox.
 * - environment is a script we will evaluate to obtain the sandbox environment,
 *   or NULL to use an empty environment. If supplied, the script should
 *   evaluate to a table, which we will use as the environment. This environment
 *   has access to the Lua standard library.
 * Returns: a Lua state, which you can use to inspect the results of the
 * sandboxed evaluation.
 *
 */
int sandbox_run(sandbox_t *sandbox,
                const char *filename,
                const char *environment);

#endif
