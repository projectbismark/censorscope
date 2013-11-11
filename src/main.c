#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "sandbox.h"
#include "scheduling.h"
#include "transport.h"

int main(int argc, char **argv) {
    fprintf(stderr, "Starting now.\n");

    transport_t transport;
    if (transport_init(&transport, "rsync")) {
        fprintf(stderr, "Error initializing transport\n");
        return 1;
    }
    if (transport_download(&transport)) {
        /* return */
    }

    /* Load the experiments configuration from sandbox/main.lua. */
    sandbox_t sandbox;
    if (sandbox_init(&sandbox, 102400, 1024)) {
        fprintf(stderr, "Error initializing sandbox.\n");
        return 1;
    }
    if (sandbox_run(&sandbox, "sandbox/main.lua", NULL)) {
        fprintf(stderr, "Error running code in sandbox.\n");
        return 1;
    }
    experiment_schedules_t schedules;
    if (experiment_schedules_init(&schedules, sandbox.L, 1)) {
        fprintf(stderr, "Error initializing experiments schedule.\n");
        return 1;
    }
    if (sandbox_destroy(&sandbox)) {
        fprintf(stderr, "Error destroying sandbox.\n");
        return 1;
    }

    /* Now run all the experiments in an event loop. */
    if (experiments_schedules_run(&schedules)) {
        fprintf(stderr, "Error running experiments.\n");
        return 1;
    }

    if (experiment_schedules_destroy(&schedules)) {
        fprintf(stderr, "Error destroying experiments.\n");
        return 1;
    }

    if (transport_upload(&transport)) {
      /* return */
    }

    if (transport_destroy(&transport)) {
        fprintf(stderr, "Error destroying transport\n");
        return 1;
    }

    fprintf(stderr, "Ran successfully.\n");

    return 0;
}
