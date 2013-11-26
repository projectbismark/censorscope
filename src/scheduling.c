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
#include "logging.h"
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

static void experiment_callback(evutil_socket_t fd, short what, void *arg) {
    experiment_schedule_t *schedule = arg;
    if (schedule->num_runs == 0) {
        event_del(schedule->ev);
    } else {
        schedule->num_runs--;
    }
    experiment_run(&schedule->experiment);
}

static int experiment_schedule_init(experiment_schedule_t *schedule,
                                    censorscope_options_t *options,
                                    struct event_base *base,
                                    const char *name,
                                    int interval_seconds,
                                    int num_runs) {
    schedule->interval_seconds = interval_seconds;
    schedule->num_runs = num_runs;

    if (schedule->num_runs <= 0) {
        schedule->ev = NULL;
        return 0;
    }

    schedule->ev = event_new(base,
                             -1,
                             EV_TIMEOUT | EV_PERSIST,
                             experiment_callback,
                             schedule);
    if (!schedule->ev) {
        log_error("error calling event_new");
        return -1;
    }

    if (experiment_init(&schedule->experiment, name, options, base)) {
        log_error("error initializing experiment");
        return -1;
    }

    struct timeval next_run;
    next_run.tv_sec = interval_seconds;
    next_run.tv_usec = 0;
    if (event_add(schedule->ev, &next_run)) {
        log_error("error adding event.");
        return -1;
    }

    log_info("loaded experiment '%s' with interval %ld to run %ld times",
             name,
             interval_seconds,
             num_runs);

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
        log_error("calloc error: %m");
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
            log_error("error initializing experiment");
            lua_pop(L, 3);  /* Pop value, key, and experiments table. */
            return -1;
        }

        lua_pop(L, 1);  /* Pop value. Leave key for lua_next. */
        ++i;
    }

    lua_pop(L, 1);  /* Pop the experiments table. */

    return 0;
}

int experiment_schedules_stop_pending(experiment_schedules_t *schedules) {
    int return_value = 0;
    for (int i = 0; i < schedules->count; ++i) {
        if (!schedules->schedules[i].ev) {
            continue;
        }
        if (event_del(schedules->schedules[i].ev)) {
            log_error("error deleting event");
            return_value = -1;
        }
    }
    return return_value;
}

int experiment_schedules_destroy(experiment_schedules_t *schedules) {
    for (int i = 0; i < schedules->count; ++i) {
        experiment_destroy(&schedules->schedules[i].experiment);
        if (schedules->schedules[i].ev) {
            event_free(schedules->schedules[i].ev);
        }
    }
    free(schedules->schedules);
    return 0;
}
