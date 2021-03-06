#include "options.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lua.h"
#include "lualib.h"

#include "logging.h"
#include "../ext/ini.h"

#ifndef DEFAULT_SANDBOX_DIR
#define DEFAULT_SANDBOX_DIR "sandbox"
#endif

#ifndef DEFAULT_LUASRC_DIR
#define DEFAULT_LUASRC_DIR "luasrc"
#endif

#ifndef DEFAULT_RESULTS_DIR
#define DEFAULT_RESULTS_DIR "results"
#endif

#ifndef DEFAULT_MAX_MEMORY
#define DEFAULT_MAX_MEMORY 0
#endif

#ifndef DEFAULT_MAX_INSTRUCTIONS
#define DEFAULT_MAX_INSTRUCTIONS 0
#endif

#ifndef DEFAULT_DOWNLOAD_TRANSPORT
#define DEFAULT_DOWNLOAD_TRANSPORT "rsync"
#endif

#ifndef DEFAULT_UPLOAD_TRANSPORT
#define DEFAULT_UPLOAD_TRANSPORT "rsync"
#endif

#ifndef DEFAULT_EXPERIMENT_TIMEOUT
#define DEFAULT_EXPERIMENT_TIMEOUT 60
#endif

#ifndef CONFIGURATION_PATH
#define CONFIGURATION_PATH "censorscope.conf"
#endif

static void print_usage(const char *program) {
    const char *usage_string =
        "Usage: %s [options]\n"
        "  -d --download-transport <transport> (default: \"%s\")\n"
        "  -h --help\n"
        "  -i --max-instructions <instructions> (default: %ld)\n"
        "  -l --luasrc-dir <path> (default: \"%s\")\n"
        "  -m --max-memory <bytes> (default: %ld)\n"
        "  -r --results-dir <path> (default: \"%s\")\n"
        "  -s --sandbox-dir <path> (default: \"%s\")\n"
        "  -t --experiment-timeout <seconds> (default: %d seconds)\n"
        "  -u --upload-transport <transport> (default: \"%s\")\n"
        "  -y --synchronous (for debugging only)\n";
    fprintf(stderr,
            usage_string,
            program,
            DEFAULT_DOWNLOAD_TRANSPORT,
            DEFAULT_MAX_INSTRUCTIONS,
            DEFAULT_LUASRC_DIR,
            DEFAULT_MAX_MEMORY,
            DEFAULT_RESULTS_DIR,
            DEFAULT_SANDBOX_DIR,
            DEFAULT_EXPERIMENT_TIMEOUT,
            DEFAULT_UPLOAD_TRANSPORT);
}

static int config_file_handler(void *user, const char *section,
                               const char* name, const char *value) {

    censorscope_options_t *options = (censorscope_options_t *) user;

    char *first_invalid;
    errno = 0;

    log_debug("got value '%s' for option '%s' from config file", value, name);

    if (strcmp(name, "sandbox-dir") == 0) {
        options->sandbox_dir = strdup(value);
    } else if (strcmp(name, "luasrc-dir") == 0) {
        options->luasrc_dir = strdup(value);
    } else if (strcmp(name, "results-dir") == 0) {
        options->results_dir = strdup(value);
    } else if (strcmp(name, "max-memory") == 0) {
        options->max_memory = strtol(value, &first_invalid, 10);
        if (errno) {
            log_error("strtol error: %m");
            censorscope_options_destroy(options);
            return 0;
        }
        if (first_invalid[0] != '\0') {
            log_error("invalid max memory: not a number");
            censorscope_options_destroy(options);
            return 0;
        }
    } else if (strcmp(name, "max-instructions") == 0) {
        options->max_instructions = strtol(value, &first_invalid, 10);
        if (errno) {
            log_error("strtol error: %m");
            censorscope_options_destroy(options);
            return 0;
        }
        if (first_invalid[0] != '\0') {
            log_error("invalid instruction count: not a number");
            censorscope_options_destroy(options);
            return 0;
        }
    } else if (strcmp(name, "download-transport") == 0) {
        options->download_transport = strdup(value);
    } else if (strcmp(name, "upload-transport") == 0) {
        options->upload_transport = strdup(value);
    } else if (strcmp(name, "synchronous") == 0) {
        options->synchronous = atoi(value);
    } else if (strcmp(name, "experiment-timeout-seconds") == 0) {
        options->experiment_timeout_seconds = strtol(value, &first_invalid, 10);
        if (errno) {
            log_error("strtol error: %m");
            censorscope_options_destroy(options);
            return 0;
        }
        if (first_invalid[0] != '\0') {
            log_error("invalid experiment timeout: ");
            censorscope_options_destroy(options);
            return 0;
        }
    } else {
        log_error("invalid configuration option: '%s'", name);
        return 0;
    }

    return 1;
}

static int parse_config_file(censorscope_options_t *options) {
    int error = ini_parse(CONFIGURATION_PATH, config_file_handler, options);

    if (error == 0) {
        log_info("successfully loaded configuration from '%s'",
                 CONFIGURATION_PATH);
        return 0;
    } else if (error == -1) {
        log_error("error opening configuration file '%s'", CONFIGURATION_PATH);
        return -1;
    } else if (error == -2) {
        log_error("error allocating memory for configuration file '%s'",
                  CONFIGURATION_PATH);
        return -1;
    } else if (error > 0) {
        log_error("error in configution file '%s'; first error on line %d",
                  CONFIGURATION_PATH,
                  error);
        return -1;
    } else {
        log_error("unknown error loading configution file '%s': %d",
                  CONFIGURATION_PATH,
                  error);
        return -1;
    }
}

static int set_default_options(censorscope_options_t *options) {
    options->sandbox_dir = strdup(DEFAULT_SANDBOX_DIR);
    if (!options->sandbox_dir) {
        log_error("strdup error: %m");
        return -1;
    }
    options->luasrc_dir = strdup(DEFAULT_LUASRC_DIR);
    if (!options->luasrc_dir) {
        free(options->sandbox_dir);
        log_error("strdup error: %m");
        return -1;
    }
    options->results_dir = strdup(DEFAULT_RESULTS_DIR);
    if (!options->results_dir) {
        free(options->luasrc_dir);
        free(options->sandbox_dir);
        log_error("strdup error: %m");
        return -1;
    }
    options->max_memory = DEFAULT_MAX_MEMORY;
    options->max_instructions = DEFAULT_MAX_INSTRUCTIONS;
    options->download_transport = strdup(DEFAULT_DOWNLOAD_TRANSPORT);
    if (!options->download_transport) {
        free(options->results_dir);
        free(options->luasrc_dir);
        free(options->sandbox_dir);
        log_error("strdup error: %m");
        return -1;
    }
    options->upload_transport = strdup(DEFAULT_UPLOAD_TRANSPORT);
    if (!options->upload_transport) {
        free(options->download_transport);
        free(options->results_dir);
        free(options->luasrc_dir);
        free(options->sandbox_dir);
        log_error("strdup error: %m");
        return -1;
    }
    options->synchronous = 0;
    options->experiment_timeout_seconds = DEFAULT_EXPERIMENT_TIMEOUT;

    return 0;
}

static int parse_cli_options(censorscope_options_t *options,
                             int argc,
                             char **argv) {
    const char *short_options = "d:hi:l:m:r:s:t:u:y";
    const struct option long_options[] = {
        {"download-transport", 1, NULL, 'd'},
        {"help", 0, NULL, 'h'},
        {"max-instructions", 1, NULL, 'i'},
        {"luasrc-dir", 1, NULL, 'l'},
        {"max-memory", 1, NULL, 'm'},
        {"results-dir", 1, NULL, 'r'},
        {"sandbox-dir", 1, NULL, 's'},
        {"experiment-timeout", 1, NULL, 't'},
        {"upload-transport", 1, NULL, 'u'},
        {"synchronous", 0, NULL, 'y'},
        {0, 0, 0, 0}
    };
    for (;;) {
        int option_index = 0;
        int c = getopt_long(argc,
                            argv,
                            short_options,
                            long_options,
                            &option_index);
        if (c == -1) {
            log_debug("done parsing options");
            break;
        }

        log_debug("got option %c", c);

        char *first_invalid;
        switch (c) {
        case 'd':
            free(options->download_transport);
            options->download_transport = strdup(optarg);
            if (!options->download_transport) {
                log_error("strdup error: %m");
                censorscope_options_destroy(options);
                return -1;
            }
            break;

        case 'h':
            print_usage(argv[0]);
            censorscope_options_destroy(options);
            exit(EXIT_SUCCESS);

        case 'i':
            errno = 0;
            options->max_instructions = strtol(optarg, &first_invalid, 10);
            if (errno) {
                log_error("strtol error: %m");
                censorscope_options_destroy(options);
                return -1;
            }
            if (first_invalid[0] != '\0') {
                log_error("invalid instruction count: not a number");
                censorscope_options_destroy(options);
                return -1;
            }
            break;

        case 'l':
            free(options->luasrc_dir);
            options->luasrc_dir = strdup(optarg);
            if (!options->luasrc_dir) {
                log_error("strdup error: %m");
                censorscope_options_destroy(options);
                return -1;
            }
            break;

        case 'm':
            errno = 0;
            options->max_memory = strtol(optarg, &first_invalid, 10);
            if (errno) {
                log_error("strtol error: %m");
                censorscope_options_destroy(options);
                return -1;
            }
            if (first_invalid[0] != '\0') {
                log_error("invalid max memory: not a number");
                censorscope_options_destroy(options);
                return -1;
            }
            break;

        case 'r':
            free(options->results_dir);
            options->results_dir = strdup(optarg);
            if (!options->results_dir) {
                log_error("strdup error: %m");
                censorscope_options_destroy(options);
                return -1;
            }
            break;

        case 's':
            free(options->sandbox_dir);
            options->sandbox_dir = strdup(optarg);
            if (!options->sandbox_dir) {
                log_error("strdup error: %m");
                censorscope_options_destroy(options);
                return -1;
            }
            break;

        case 't':
            errno = 0;
            options->experiment_timeout_seconds = strtol(optarg,
                                                         &first_invalid,
                                                         10);
            if (errno) {
                log_error("strtol error: %m");
                censorscope_options_destroy(options);
                return -1;
            }
            if (first_invalid[0] != '\0') {
                log_error("invalid experiment timeout: ");
                censorscope_options_destroy(options);
                return -1;
            }
            break;

        case 'u':
            free(options->upload_transport);
            options->upload_transport = strdup(optarg);
            if (!options->upload_transport) {
                log_error("strdup error: %m");
                censorscope_options_destroy(options);
                return -1;
            }
            break;

        case 'y':
            options->synchronous = 1;
            break;

        default:
            log_error("invalid option");
            print_usage(argv[0]);
            censorscope_options_destroy(options);
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}

int censorscope_options_init(censorscope_options_t *options,
                             int argc,
                             char **argv) {
    /* set default options first */
    if (set_default_options(options)) {
        /* we've already logged the error */
        return -1;
    }

    /* parse the config file */
    if (parse_config_file(options)) {
        /* we've already logged the error */
        return -1;
    }

    /* parse the command line arguments */
    if (parse_cli_options(options, argc, argv)) {
        /* we've already logged the error */
        return -1;
    }

    return 0;
}

int censorscope_options_lua(const censorscope_options_t *options,
                            lua_State *L) {
    lua_pushstring(L, options->sandbox_dir);
    lua_setfield(L, -2, "sandbox_dir");
    lua_pushstring(L, options->luasrc_dir);
    lua_setfield(L, -2, "luasrc_dir");
    lua_pushstring(L, options->results_dir);
    lua_setfield(L, -2, "results_dir");
    lua_pushnumber(L, options->max_memory);
    lua_setfield(L, -2, "max_memory");
    lua_pushnumber(L, options->max_instructions);
    lua_setfield(L, -2, "max_instructions");
    lua_pushstring(L, options->download_transport);
    lua_setfield(L, -2, "download_transport");
    lua_pushstring(L, options->upload_transport);
    lua_setfield(L, -2, "upload_transport");
    return 0;
}

int censorscope_options_destroy(censorscope_options_t *options) {
    free(options->luasrc_dir);
    free(options->sandbox_dir);
    free(options->results_dir);
    free(options->download_transport);
    free(options->upload_transport);
    return 0;
}
