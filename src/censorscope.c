#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <event2/event.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "options.h"
#include "sandbox.h"
#include "scheduling.h"
#include "transport.h"
#include "util.h"

int main(int argc, char **argv) {
    censorscope_options_t options;
    if (censorscope_options_init(&options, argc, argv)) {
        fprintf(stderr, "Error parsing flags.\n");
        return 1;
    }

    struct event_base *base = event_base_new();
    if (!base) {
        fprintf(stderr, "Could not initialise libevent\n");
        return 1;
    }

    fprintf(stderr, "Starting now.\n");

    transport_t transport;
    if (transport_init(&transport, &options, options.download_transport)) {
        fprintf(stderr, "Error initializing transport\n");
        return 1;
    }
    if (transport_download(&transport)) {
        /* return */
    }
    if (transport_destroy(&transport)) {
        fprintf(stderr, "Error destroying transport\n");
        return 1;
    }

    /* Load the experiments configuration from sandbox/main.lua. */
    sandbox_t sandbox;
    if (sandbox_init(&sandbox, "main", &options)) {
        fprintf(stderr, "Error initializing sandbox.\n");
        return 1;
    }
    char *main_filename = sprintf_malloc("%s/main.lua", options.sandbox_dir);
    if (!main_filename) {
        return 1;
    }
    if (sandbox_run(&sandbox, main_filename, NULL)) {
        fprintf(stderr, "Error running code in sandbox.\n");
        free(main_filename);
        return 1;
    }
    free(main_filename);
    experiment_schedules_t schedules;
    if (experiment_schedules_init(&schedules, &options, base, sandbox.L, 1)) {
        fprintf(stderr, "Error initializing experiments schedule.\n");
        return 1;
    }
    if (sandbox_destroy(&sandbox)) {
        fprintf(stderr, "Error destroying sandbox.\n");
        return 1;
    }

    /* start the event loop */
    event_base_dispatch(base);

    if (transport_init(&transport, &options, options.upload_transport)) {
        fprintf(stderr, "Error initializing transport\n");
        return 1;
    }
    if (transport_upload(&transport)) {
      /* return */
    }
    if (transport_destroy(&transport)) {
        fprintf(stderr, "Error destroying transport\n");
        return 1;
    }

    if (experiment_schedules_destroy(&schedules)) {
        fprintf(stderr, "Error destroying schedules.\n");
        return 1;
    }
    event_base_free(base);
    if (censorscope_options_destroy(&options)) {
        fprintf(stderr, "Error destroying options.\n");
        return 1;
    }

    fprintf(stdout, "Ran successfully.\n");
    return 0;
}
