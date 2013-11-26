#ifndef CENSORSCOPE_TERMINATION_H
#define CENSORSCOPE_TERMINATION_H

#include "scheduling.h"

struct event_base;

int add_termination_handlers(struct event_base *base,
                             experiment_schedules_t *schedules);

#endif
