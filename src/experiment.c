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

int experiment_init(experiment_t *experiment,
                    const char *name,
                    censorscope_options_t *options) {
    experiment->options = options;

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

int experiment_run(experiment_t *experiment) {
    /* Set OS limits on the child. */
    set_limits(experiment->options);

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

int experiment_destroy(experiment_t *experiment) {
    free(experiment->name);
    free(experiment->path);
    return 0;
}
