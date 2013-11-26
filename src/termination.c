#include "termination.h"

#include <signal.h>

#include <event2/event.h>

#include "scheduling.h"
#include "logging.h"

static struct event *sigterm_event;
static struct event *sigint_event;

static void handler(evutil_socket_t fd, short what, void *arg) {
    log_info("entering lame duck mode; next signal will kill immediately");
    experiment_schedules_t *schedules = (experiment_schedules_t *)arg;
    experiment_schedules_stop_pending(schedules);
    evsignal_del(sigterm_event);
    evsignal_del(sigint_event);
}

static int add_handler(struct event_base *base,
                       experiment_schedules_t *schedules,
                       struct event **ev,
                       int signum) {
    *ev = evsignal_new(base, signum, handler, schedules);
    if (!*ev) {
        log_error("error creating termination event");
        return 1;
    }
    if (evsignal_add(*ev, NULL)) {
        log_error("error adding termination event");
        return 1;
    }
    return 0;
}

int add_termination_handlers(struct event_base *base,
                             experiment_schedules_t *schedules) {
    if (add_handler(base, schedules, &sigterm_event, SIGTERM)) {
        return -1;
    }
    if (add_handler(base, schedules, &sigint_event, SIGINT)) {
        return -1;
    }
    return 0;
}
