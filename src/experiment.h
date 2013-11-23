#ifndef CENSORSCOPE_EXPERIMENT_H
#define CENSORSCOPE_EXPERIMENT_H

#include "options.h"

struct event_base;

typedef struct {
    char *name;
    char *path;
    censorscope_options_t *options;
    struct event_base *base;
} experiment_t;

int experiment_init(experiment_t *experiment,
                    const char *name,
                    censorscope_options_t *options,
                    struct event_base *base);

void experiment_run(experiment_t *experiment);

int experiment_destroy(experiment_t *experiment);

#endif
