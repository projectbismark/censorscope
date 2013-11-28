#include "subprocesses.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <event2/event.h>

#include "logging.h"

/* This tracks state for a single subprocess. */
typedef struct child_info {
    /* The pid of the subprocess. */
    pid_t pid;
    /* The subprocess's termination event. */
    struct event *ev;
    /* A link back to the global subprocesses structure. We need this so a
     * child_info_t can remove itself for the subprocesses list. */
    subprocesses_t *subprocesses;
} child_info_t;

/* For every subprocess, we register an event that kills that subprocess after a
 * timeout. This function removes that event for one subprocess. You should
 * call this function immediately after you reap the child using waitpid.
 *
 */
static void remove_subprocess_killer(subprocesses_t *subprocesses, pid_t pid) {
    int pids_matched = 0;
    for (int i = subprocesses->count - 1; i >= 0; --i) {
        if (subprocesses->children[i].pid != pid) {
            continue;
        }
        ++pids_matched;

        /* This is the important line. */
        event_free(subprocesses->children[i].ev);

        /* Remove the child by swapping it to the end of the list of children
         * and decrementing the list length. This is ok to do because the list
         * of children is unordered. */
        subprocesses->children[i] = subprocesses->children[subprocesses->count - 1];
        --subprocesses->count;
    }
    assert(pids_matched == 1);  /* Sanity check. */

    /* When there are no subprocesses outstanding, deactivate the SIGCHLD
     * handler. Doing so lets event_base_dispatch exit when there are no
     * remaining events to handle. If we didn't remove this signal handler, then
     * event_base_dispatch could hang around indefinitely waiting for SIGCHLD
     * events that will never come. */
    if (subprocesses->count == 0) {
        if (evsignal_del(subprocesses->sigchld_event)) {
            log_error("error removing SIGCHLD event handler");
        }
    }
}

/* This is a callback that kills a child process after a timeout. */
static void kill_subprocess(evutil_socket_t fd, short what, void *arg) {
    child_info_t *info = (child_info_t *)arg;

    int pid = waitpid(info->pid, NULL, WNOHANG);
    if (pid == -1) {
        log_error("pid %d is not a child; has it already been reaped?",
                  info->pid);
        return;
    }

    /* The child process might have already exited naturally. We will normally
     * wait on such a processes in sigchld_handler, but there's a chance that
     * the process exited after libevent dispatched to kill_subprocess. */
    if (pid == info->pid) {
        log_info("pid %d has already exited", info->pid);
        remove_subprocess_killer(info->subprocesses, info->pid);
        return;
    }

    /* Kill the child. We kill the entire process group to minimize the
     * possibility of stray sub-sub-processes. This works because all children
     * call setsid upon forking. */
    log_info("pid %d has not exited yet; killing it now", info->pid);
    pid_t process_group = -1 * info->pid;
    if (kill(process_group, SIGKILL) || kill(info->pid, SIGKILL)) {
        log_error("kill: %m");
        return;
    }
    /* Reap the pid of the newly killed child. Run in a loop
     * in case a signal interrupts the call to waitpid. */
    do {
        pid = waitpid(info->pid, NULL, 0);
    } while(pid == -1 && errno == EINTR);
    if (pid == -1) {
        log_error("waitpid: %m");
        return;
    }
    remove_subprocess_killer(info->subprocesses, info->pid);
}

static void sigchld_handler(evutil_socket_t fd, short what, void *arg) {
    subprocesses_t *subprocesses = (subprocesses_t *)arg;

    /* We might only receive a single SIGCHLD for several exiting children, so
     * we try to reap as many children as possible. */
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        log_info("reaping pid %d", pid);
        remove_subprocess_killer(subprocesses, pid);
    }
    if (pid < 0 && errno != ECHILD) {  /* ECHILD isn't an error in this case. */
        log_error("waitpid: %m");
    }
}

int subprocesses_init(subprocesses_t *subprocesses, struct event_base *base) {
    subprocesses->children = NULL;
    subprocesses->count = 0;
    subprocesses->capacity = 0;
    subprocesses->base = base;

    subprocesses->sigchld_event = evsignal_new(base,
                                               SIGCHLD,
                                               sigchld_handler,
                                               subprocesses);
    if (!subprocesses->sigchld_event) {
        log_error("error creating SIGCHLD event handler");
        return -1;
    }
    return 0;
}

int subprocesses_fork(subprocesses_t *subprocesses, time_t timeout_seconds) {
    /* Grow the list of subprocesses as needed. */
    if (subprocesses->count + 1 > subprocesses->capacity) {
        subprocesses->capacity = (subprocesses->capacity + 1) * 2;
        subprocesses->children = realloc(
                subprocesses->children,
                subprocesses->capacity * sizeof(child_info_t));
        if (!subprocesses->children) {
            log_error("realloc: %m");
            return -1;
        }
    }

    /* Ensure subprocess killer gets run after a timeout. It's ok to
     * evsignal_add multiple times on the same event. Install this handler
     * before forking just in case the child exits really quickly. */
    if (evsignal_add(subprocesses->sigchld_event, NULL)) {
        log_error("error adding SIGCHLD event handler");
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        log_error("fork: %m");
        return -1;
    } else if (pid == 0) {
        /* We are the child process. */

        /* Put the child in its own process group so we can kill the process
         * group as a whole, which will hopefully kill all the child's children
         * as well. */
        if (setsid() == -1) {
            log_error("setsid: %m");
            exit(EXIT_FAILURE);
        }
        return 0;
    }

    /* We are the parent process. */
    log_info("spawned child pid %d", pid);

    /* Schedule an event to kill the child after a timeout. */
    child_info_t *info = &subprocesses->children[subprocesses->count];
    info->pid = pid;
    info->ev = event_new(subprocesses->base,
                         -1,
                         EV_TIMEOUT,
                         kill_subprocess,
                         info);
    if (!info->ev) {
        log_error("error creating timeout event");
        return -1;
    }
    struct timeval kill_timeout = { timeout_seconds, 0 };
    if (event_add(info->ev, &kill_timeout)) {
        log_info("error calling event_add");
        event_free(info->ev);
        return -1;
    }
    ++subprocesses->count;
    return pid;
}

int subprocesses_destroy(subprocesses_t *subprocesses) {
    if (subprocesses->count != 0) {
        log_error("not all subprocesses have exited");
        return -1;
    }
    free(subprocesses->children);
    event_free(subprocesses->sigchld_event);
    return 0;
}
