#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <event2/event.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "logging.h"
#include "options.h"
#include "sandbox.h"
#include "scheduling.h"
#include "transport.h"
#include "util.h"

int main(int argc, char **argv) {
    logging_init();

    censorscope_options_t options;
    if (censorscope_options_init(&options, argc, argv)) {
        log_error("error parsing flags");
        return 1;
    }

    struct event_base *base = event_base_new();
    if (!base) {
        log_error("could not initialise libevent");
        return 1;
    }

    transport_t transport;
    if (transport_init(&transport, &options, options.download_transport)) {
        log_error("error initializing transport");
        return 1;
    }
    if (transport_download(&transport)) {
        log_error("error synchronizing sandbox");
    }
    if (transport_destroy(&transport)) {
        log_error("error destroying transport");
        return 1;
    }

    /* Load the experiments configuration from sandbox/main.lua. */
    sandbox_t sandbox;
    if (sandbox_init(&sandbox, "main", &options)) {
        log_error("error initializing sandbox");
        return 1;
    }
    char *main_filename = sprintf_malloc("%s/main.lua", options.sandbox_dir);
    if (!main_filename) {
        log_error("error allocating main_filename");
        return 1;
    }
    if (sandbox_run(&sandbox, main_filename, NULL)) {
        log_error("error running code in sandbox");
        free(main_filename);
        return 1;
    }
    free(main_filename);
    experiment_schedules_t schedules;
    if (experiment_schedules_init(&schedules, &options, base, sandbox.L, 1)) {
        log_error("error initializing experiments schedule");
        return 1;
    }
    if (sandbox_destroy(&sandbox)) {
        log_error("error destroying sandbox");
        return 1;
    }

    /* start the event loop */
    if (event_base_dispatch(base) == -1) {
        log_error("error calling event_base_dispatch");
    } else {
        log_info("no more events to dispatch");
    }

    if (transport_init(&transport, &options, options.upload_transport)) {
        log_error("error initializing transport");
        return 1;
    }
    if (transport_upload(&transport)) {
        log_error("error uploading results");
    }
    if (transport_destroy(&transport)) {
        log_error("error destroying transport");
        return 1;
    }

    if (experiment_schedules_destroy(&schedules)) {
        log_error("error destroying schedules");
        return 1;
    }
    event_base_free(base);
    if (censorscope_options_destroy(&options)) {
        log_error("error destroying options");
        return 1;
    }

    logging_destroy();
    return 0;
}
