#ifndef _CENSORSCOPE_SANDBOX_H_
#define _CENSORSCOPE_SANDBOX_H_

#include "lua.h"

#include "options.h"

typedef struct {
    lua_State *L;
    size_t available_memory;
} sandbox_t;

/* Initialize a sandbox, which you can use to run Lua code with memory and
 * instruction count constraints.
 *
 * Arguments:
 * - name is a name for the sandbox, which will be exposed when evaluating the
 *   environment.
 * - options is the set of censorscope options, which will be exposed when
 *   evaluating the environment.
 * Returns: 0 on success, -1 on failure.
 *
 */
int sandbox_init(sandbox_t *sandbox,
                 const char *name,
                 const censorscope_options_t *options);

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
