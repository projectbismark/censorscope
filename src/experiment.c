#include "experiment.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <event2/event.h>

#include "logging.h"
#include "options.h"
#include "register.h"
#include "sandbox.h"
#include "util.h"

typedef struct {
    pid_t pid;
    struct event *ev;
} child_info_t;

int experiment_init(experiment_t *experiment,
                    const char *name,
                    censorscope_options_t *options,
                    struct event_base *base) {
    experiment->options = options;
    experiment->base = base;

    if (!is_valid_module_name(name)) {
        log_error("invalid experiment name");
        return -1;
    }

    experiment->name = strdup(name);
    if (!experiment->name) {
        log_error("strdup error: %m");
        return -1;
    }

    experiment->path = module_filename(options->sandbox_dir, name);
    if (!experiment->path) {
        log_error("strdup error: %m");
        return -1;
    }
    return 0;
}

static int set_limits(const censorscope_options_t *options) {
    rlim_t as_limit = 200*1024*1024;
    struct rlimit limits = { as_limit, as_limit };
    setrlimit(RLIMIT_AS, &limits);
    return 0;
}

static int run_child(experiment_t *experiment) {
    censorscope_options_t *options = experiment->options;

    sandbox_t sandbox;
    if (sandbox_init(&sandbox, experiment->name, options)) {
        log_error("error initializing sandbox for '%s'", experiment->path);
        return -1;
    }
    if (register_functions(options, &sandbox)) {
        log_error("error registering sandbox functions");
        sandbox_destroy(&sandbox);
        return -1;
    }
    char *filename = sprintf_malloc("%s/api.lua", options->luasrc_dir);
    if (!filename) {
        log_error("error allocating filename for '%s'", options->luasrc_dir);
        sandbox_destroy(&sandbox);
        return -1;
    }
    if (sandbox_run(&sandbox, experiment->path, filename)) {
        log_error("error running '%s'", experiment->path);
        free(filename);
        sandbox_destroy(&sandbox);
        return -1;
    }
    free(filename);
    sandbox_destroy(&sandbox);
    return 0;
}

static void kill_child(evutil_socket_t fd, short what, void *arg) {
    child_info_t *info = (child_info_t *)arg;

    event_free(info->ev);  /* This event does not recur. */

    int rc = waitpid(info->pid, NULL, WNOHANG);
    if (rc == info->pid) {
        log_info("pid %d has already exited", info->pid);
        free(info);
        return;
    } else if (rc == -1) {
        log_error("pid %d is not a child; has it already been reaped?",
                  info->pid);
        free(info);
        return;
    }
    log_info("pid %d has not exited yet; killing it now", info->pid);
    pid_t process_group = -1 * info->pid;
    if (kill(process_group, SIGKILL)) {
        log_error("kill: %m");
        free(info);
        return;
    }
    /* Reap the pid of the newly killed child. */
    if (waitpid(info->pid, NULL, 0) == -1) {
        log_error("waitpid: %m");
    }
    free(info);
}

void experiment_run(experiment_t *experiment) {
    pid_t pid = fork();
    if (pid < 0) {
        log_error("fork: %m");
        return;
    } else if (pid == 0) {
        /* We are now the child process. */

        /* Put the child in its own process group so we can kill the process
         * group as a whole, which will hopefully kill all the child's children
         * as well. */
        if (setsid() == -1) {
            log_error("setsid: %m");
            exit(EXIT_FAILURE);
        }

        /* Set OS limits on the child. */
        set_limits(experiment->options);

        if (run_child(experiment)) {
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }

    /* We are still in the parent. */

    log_info("spawned child pid %d", pid);

    child_info_t *info = malloc(sizeof(child_info_t));
    if (!info) {
        log_error("malloc: %m");
        return;
    }
    struct event *ev = event_new(experiment->base,
                                 -1,
                                 EV_TIMEOUT,
                                 kill_child,
                                 info);
    if (!ev) {
        log_error("error creating timeout event");
        free(info);
        return;
    }
    info->pid = pid;
    info->ev = ev;
    struct timeval kill_timeout = {
        experiment->options->experiment_timeout_seconds, 0 };
    if (event_add(ev, &kill_timeout)) {
        log_info("error calling event_add");
        event_free(ev);
        free(info);
        return;
    }
    if (experiment->options->synchronous) {
        log_info("running experiment synchronously");
        waitpid(pid, NULL, 0);
    }
    return;
}

int experiment_destroy(experiment_t *experiment) {
    free(experiment->name);
    free(experiment->path);
    return 0;
}
