#ifndef CENSORSCOPE_SCHEDULING_H
#define CENSORSCOPE_SCHEDULING_H

#include <time.h>

#include "lua.h"

typedef struct {
    char *experiment;
    lua_Integer interval;
    lua_Integer num_runs;

    time_t next_run;
    char *path;
} experiment_schedule_t;

typedef struct {
    experiment_schedule_t *schedules;
    int count;
} experiment_schedules_t;

/* Initialize an experiments schedule.
 *
 * The top of the provided Lua stack must contain a table of censorscope
 * settings. This usually comes from sandbox/main.lua.
 *
 */
int experiment_schedules_init(experiment_schedules_t *schedules,
                              lua_State *L,
                              int table_index);

int experiment_schedules_destroy(experiment_schedules_t *schedules);

int experiments_schedules_run(experiment_schedules_t *schedules);

#endif
