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

#include "censorscope.h"
#include "dns.h"
#include "register.h"
#include "sandbox.h"
#include "util.h"

#define NEVER_RUN -1
#define RUN_NOW 0
#define SECONDS_PER_MINUTE 60

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
    if (sandbox_init(&sandbox, 102400, 102400)) {
        fprintf(stderr, "Error initializing sandbox for '%s'\n", schedule->path);
    }
    if (register_functions(&sandbox)) {
        fprintf(stderr, "Error registering sandbox functions\n");
    }
    if (sandbox_run(&sandbox, schedule->path, "luasrc/api.lua")) {
        fprintf(stderr, "Error running '%s'\n", schedule->path);
    }
    if (sandbox_destroy(&sandbox)) {
        fprintf(stderr, "Error destorying sandbox for '%s'\n", schedule->path);
    }
}

int experiment_schedules_init(experiment_schedules_t *schedules,
                              lua_State *L,
                              int table_index) {
    lua_getfield(L, table_index, "experiments");

    schedules->count = count_table_keys(L, -1);
    schedules->schedules = calloc(schedules->count,
                                  sizeof(experiment_schedule_t));
    if (!schedules->schedules) {
        perror("error allocating experiment schedules");
        return -1;
    }

    int i = 0;
    lua_pushnil(L);  /* first key */
    while (lua_next(L, -2) != 0) {
        experiment_schedule_t *schedule = &schedules->schedules[i];
        schedule->experiment = strdup(luaL_checkstring(L, -2));
        if (!schedule->experiment) {
            perror("strdup");
            return -1;
        }
        if (!is_valid_module_name(schedule->experiment)) {
            fprintf(stderr, "invalid experiment name\n");
            return -1;
        }

        lua_getfield(L, -1, "interval");
        schedule->interval = luaL_checkinteger(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, -1, "num_runs");
        schedule->num_runs = luaL_checkinteger(L, -1);
        lua_pop(L, 1);

        fprintf(stderr,
                "Loaded experiment '%s' with interval %ld to run %ld times.\n",
                schedule->experiment,
                schedule->interval,
                schedule->num_runs);

        schedule->path = module_filename(schedule->experiment);
        if (!schedule->path) {
            perror("strdup");
            return -1;
        }

        /* schedule only experiments that we need to run */
        if (schedule->num_runs > 0) {
            schedule->ev = event_new(base, -1, EV_TIMEOUT|EV_PERSIST,
                                     run_experiment, schedule);
            if (!schedule->ev) {
                fprintf(stderr, "Error calling event_new\n");
                return -1;
            }

            struct timeval next_run;
            next_run.tv_sec = schedule->interval * SECONDS_PER_MINUTE;
            next_run.tv_usec = 0;
            if (event_add(schedule->ev, &next_run)) {
                fprintf(stderr, "Error adding event.\n");
                return -1;
            }
        } else {
            schedule->ev = NULL;
        }

        lua_pop(L, 1);
        ++i;
    }
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
