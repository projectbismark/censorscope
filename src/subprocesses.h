#ifndef CENSORSCOPE_SUBPROCESSES_H
#define CENSORSCOPE_SUBPROCESSES_H

#include <time.h>

#include "options.h"

struct child_info_t;
struct event;
struct event_base;

/* This tracks state of subprocesses so we can terminate them after a imeout. */
typedef struct {
    /* An array of 'capacity' elements, the first 'count' of which are valid. */
    struct child_info *children;
    int count, capacity;

    /* We need this to add and remove timeout events. */
    struct event_base *base;
    /* This is the event for the SIGCHLD handler, which we use to remove timeout
     * events when subprocesses exit early. */
    struct event *sigchld_event;
} subprocesses_t;

/* Initialize state needed to manage subprocesses.
 *
 * Arguments:
 * - subprocesses is the state to initialize.
 * - base is a libevent event base that we will use to schedule kill events for
 *   spawn subprocesses.
 * Returns 0 on success and -1 on failure.
 *
 */
int subprocesses_init(subprocesses_t *subprocesses, struct event_base *base);

/* Fork this process such that the parent subprocess will kill the child if it
 * hasn't exited after a timeout.
 *
 * Arguments:
 * - subprocesses is the state used to schedule kill events.
 * - timeout_seconds; the parent will kill the child after this many seconds.
 * Returns -1 on error, 0 to the child on success, and a positive integer to the
 * parent on success.
 *
 */
int subprocesses_fork(subprocesses_t *subprocesses, time_t timeout_seconds);

/* Destroy the subprocess state. All subprocesses must have exited before
 * calling this function.
 *
 * Returns 0 on sucess and -1 if there were errors.
 *
 */
int subprocesses_destroy(subprocesses_t *subprocesses);

#endif
