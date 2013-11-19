#include "scheduling.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <event2/event.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "dns.h"
#include "options.h"
#include "register.h"
#include "sandbox.h"
#include "util.h"

/* Return the total number of keys in a table. Adapted from
 * http://www.lua.org/manual/5.1/manual.html#lua_next
 *
 */
static int count_table_keys(lua_State *L, int table_index) {
    if (table_index < 0) {
        /* If table_index is a relative index, then offset it by 1 because we
         * will be putting an additional element on the top of the stack before
         * referencing this index. */
        --table_index;
    }

    int count = 0;
    lua_pushnil(L);  /* first key */
    while (lua_next(L, table_index) != 0) {
        count++;
        lua_pop(L, 1);
    }
    return count;
}

static void run_experiment(evutil_socket_t fd, short what, void *arg) {
    experiment_schedule_t *schedule = arg;
    if (schedule->num_runs == 0) {
        event_del(schedule->ev);
    } else {
        schedule->num_runs--;
    }

    sandbox_t sandbox;
    if (sandbox_init(&sandbox,
                     schedule->experiment,
                     schedule->options->luasrc_dir,
                     schedule->options->max_memory,
                     schedule->options->max_instructions)) {
        fprintf(stderr, "Error initializing sandbox for '%s'\n", schedule->path);
        return;
    }
    if (register_functions(schedule->options, &sandbox)) {
        fprintf(stderr, "Error registering sandbox functions\n");
        sandbox_destroy(&sandbox);
        return;
    }
    char *filename = sprintf_malloc("%s/api.lua",
                                    schedule->options->luasrc_dir);
    if (!filename) {
        fprintf(stderr,
                "Error allocating filename for '%s'\n",
                schedule->options->luasrc_dir);
        sandbox_destroy(&sandbox);
        return;
    }
    if (sandbox_run(&sandbox, schedule->path, filename)) {
        fprintf(stderr, "Error running '%s'\n", schedule->path);
        free(filename);
        sandbox_destroy(&sandbox);
        return;
    }
    free(filename);
    if (sandbox_destroy(&sandbox)) {
        fprintf(stderr, "Error destorying sandbox for '%s'\n", schedule->path);
    }
}

static int experiment_schedule_init(experiment_schedule_t *schedule,
                                    censorscope_options_t *options,
                                    struct event_base *base,
                                    const char *name,
                                    int interval_seconds,
                                    int num_runs) {
    schedule->options = options;

    if (!is_valid_module_name(name)) {
        fprintf(stderr, "invalid experiment name\n");
        return -1;
    }

    schedule->experiment = strdup(name);
    if (!schedule->experiment) {
        perror("strdup");
        return -1;
    }
    schedule->interval_seconds = interval_seconds;
    schedule->num_runs = num_runs;

    schedule->path = module_filename(options->sandbox_dir, name);
    if (!schedule->path) {
        perror("strdup");
        return -1;
    }

    if (schedule->num_runs > 0) {
        schedule->ev = event_new(base,
                                 -1,
                                 EV_TIMEOUT | EV_PERSIST,
                                 run_experiment,
                                 schedule);
        if (!schedule->ev) {
            fprintf(stderr, "Error calling event_new\n");
            return -1;
        }

        struct timeval next_run;
        next_run.tv_sec = interval_seconds;
        next_run.tv_usec = 0;
        if (event_add(schedule->ev, &next_run)) {
            fprintf(stderr, "Error adding event.\n");
            return -1;
        }
    } else {
        schedule->ev = NULL;
    }

    fprintf(stdout,
            "Loaded experiment '%s' with interval %ld to run %ld times.\n",
            schedule->experiment,
            schedule->interval_seconds,
            schedule->num_runs);

    return 0;
}

static lua_Integer checkfield_integer(lua_State *L,
                                      int table_index,
                                      const char *key) {
    lua_getfield(L, table_index, key);
    lua_Integer value = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return value;
}

int experiment_schedules_init(experiment_schedules_t *schedules,
                              censorscope_options_t *options,
                              struct event_base *base,
                              lua_State *L,
                              int table_index) {
    lua_getfield(L, table_index, "experiments");

    schedules->count = count_table_keys(L, -1);
    schedules->schedules = calloc(schedules->count,
                                  sizeof(experiment_schedule_t));
    if (!schedules->schedules) {
        perror("error allocating experiment schedules");
        lua_pop(L, 1);  /* Pop the experiments table. */
        return -1;
    }

    int i = 0;
    lua_pushnil(L);  /* first key */
    while (lua_next(L, -2) != 0) {
        if (experiment_schedule_init(&schedules->schedules[i],
                                     options,
                                     base,
                                     luaL_checkstring(L, -2),
                                     checkfield_integer(L, -1, "interval_seconds"),
                                     checkfield_integer(L, -1, "num_runs"))) {
            fprintf(stderr, "Error initializing experiment");
            lua_pop(L, 3);  /* Pop value, key, and experiments table. */
            return -1;
        }

        lua_pop(L, 1);  /* Pop value. Leave key for lua_next. */
        ++i;
    }

    lua_pop(L, 1);  /* Pop the experiments table. */

    return 0;
}

int experiment_schedules_destroy(experiment_schedules_t *schedules) {
    for (int i = 0; i < schedules->count; ++i) {
        free(schedules->schedules[i].experiment);
        free(schedules->schedules[i].path);
        event_free(schedules->schedules[i].ev);
    }
    free(schedules->schedules);
    return 0;
}
