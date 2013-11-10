#include "sandbox.h"

#include <stdlib.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#define BYTECODE_MAGIC_NUMBER 27

/* This is an allocation function that artificially limits the pool of memory
 * available to scripts allocate.
 *
 * References:
 * http://www.lua.org/manual/5.1/manual.html#lua_Alloc
 * http://stackoverflow.com/a/9672205
 *
 */
static void *l_alloc_restricted(void *ud, void *ptr, size_t osize, size_t nsize) {
    int *available = (int *)ud;

    if (nsize == 0) {  /* free */
        free(ptr);
        *available += osize;  /* reclaim memory */
        return NULL;
    } else {  /* malloc */
        int extra_memory = nsize - osize;
        if (*available < extra_memory) {  /* too much memory in use */
            fprintf(stderr, "Out of memory!\n");
            return NULL;
        }
        ptr = realloc(ptr, nsize);
        if (ptr) {  /* reallocation successful? */
            *available -= extra_memory;
        }
        return ptr;
    }
}

/* This is a debug hook that aborts the currently running script. We use it to
 * abort a script after running a certain number of instruction cycles.
 *
 * References:
 * http://www.lua.org/manual/5.1/manual.html#lua_hook
 * http://stackoverflow.com/a/11425017
 *
 */
static void exit_hook(lua_State *L, lua_Debug *ar) {
    if (ar->event != LUA_HOOKCOUNT) {
        return;
    }
    luaL_error(L, "instruction limit reached");
}

/* Check whether an input file is Lua bytecode. We
 * do not evaluate Lua bytecode because it can escape
 * the sandbox.
 *
 * Returns 1 if the script is valid and 0 if it is not.
 *
 */
static int is_valid_lua_script(const char *filename) {
    FILE *handle = fopen(filename, "r");
    if (!handle) {
        perror(filename);
        return 0;
    }

    int first_byte = fgetc(handle);
    if (fclose(handle)) {
        perror(filename);
        return 0;
    }
    if (first_byte == 0) {
        return 0;
    }
    if (first_byte == BYTECODE_MAGIC_NUMBER) {
        fprintf(stderr, "for security, we do not evaluate Lua bytecode");
        return 0;
    }
    return 1;
}

int sandbox_init(sandbox_t *sandbox, int max_memory, int max_instructions) {
    sandbox->available_memory = max_memory;
    sandbox->L = lua_newstate(l_alloc_restricted, &sandbox->available_memory);
    if (!sandbox->L) {
        fprintf(stderr, "Error calling luaL_newstate\n");
        return -1;
    }
    lua_sethook(sandbox->L, exit_hook, LUA_MASKCOUNT, max_instructions);
    luaL_openlibs(sandbox->L);
    return 0;
}

int sandbox_destroy(sandbox_t *sandbox) {
    lua_close(sandbox->L);
    return 0;
}

int sandbox_run(sandbox_t *sandbox,
                const char *filename,
                const char *environment) {
    if (!is_valid_lua_script(filename)) {
        return -1;
    }

    /* Load (but not evaluate) the code to run in the sandbox. */
    if (luaL_loadfile(sandbox->L, filename)) {
        fprintf(stderr, "%s\n", lua_tostring(sandbox->L, -1));
        lua_close(sandbox->L);
        return -1;
    }
    if (environment) {
        /* Load (but not evaluate) the code that creates the environment. */
        if (luaL_loadfile(sandbox->L, environment)) {
            fprintf(stderr, "%s", lua_tostring(sandbox->L, -1));
            lua_close(sandbox->L);
            return -1;
        }
        /* Add SCRIPT_FILENAME to the environment so the environment can know
         * which experiment it is running under. */
        lua_pushstring(sandbox->L, filename);
        lua_setglobal(sandbox->L, "SCRIPT_FILENAME");

        /* Evaluate the environment. (Its code is at the top of the stack.) This
         * removes the the code and replaces it with the environment's table. */
        if (lua_pcall(sandbox->L, 0, 1, 0)) {
            fprintf(stderr, "%s\n", lua_tostring(sandbox->L, -1));
            lua_close(sandbox->L);
            return -1;
        }
    } else {
        /* Create an empty environment. */
        lua_newtable(sandbox->L);
    }

    /* Set the environment to use for the sandboxed evaluation. This pops the
     * environment off the top of the stack. */
    if (lua_setfenv(sandbox->L, -2) != 1) {
        fprintf(stderr, "%s\n", lua_tostring(sandbox->L, -1));
        lua_close(sandbox->L);
        return -1;
    }
    /* Evaluate the sandbox function, which is now at the top of the stack. */
    if (lua_pcall(sandbox->L, 0, 1, 0) != 0) {
        fprintf(stderr,
                "error running %s: %s\n",
                filename,
                lua_tostring(sandbox->L, -1));
        lua_close(sandbox->L);
        return -1;
    }

    return 0;
}
