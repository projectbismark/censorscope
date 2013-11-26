#ifndef CENSORSCOPE_SCHEDULING_H
#define CENSORSCOPE_SCHEDULING_H

#include "lua.h"

#include "experiment.h"
#include "options.h"

struct event_base;

typedef struct {
    lua_Integer interval_seconds;
    lua_Integer num_runs;
    struct event *ev;

    experiment_t experiment;
} experiment_schedule_t;

typedef struct {
    experiment_schedule_t *schedules;
    int count;
} experiment_schedules_t;

/* Initialize an experiments schedule.
 *
 * Arguments:
 * - schedules is the structure to initialize.
 * - base is a libevent2 event base, with which we will schedule our
 *   experiments.
 * - L is a Lua state. The stack must contain a table of censorscope settings at
 *   table_index position. This usually comes from sandbox/main.lua.
 * - table_index the stack position of censorscope settings. It can be either a
 *   relative or absolute index.
 * Returns: 0 on success, -1 on failure.
 *
 */
int experiment_schedules_init(experiment_schedules_t *schedules,
                              censorscope_options_t *options,
                              struct event_base *base,
                              lua_State *L,
                              int table_index);

int experiment_schedules_stop_pending(experiment_schedules_t *schedules);

int experiment_schedules_destroy(experiment_schedules_t *schedules);

/* Run experiments in an event loop. This function exits when there are no more
 * experiments to run.
 *
 */
int experiments_schedules_run(experiment_schedules_t *schedules);

#endif
