#include "options.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
        "  -u --upload-transport <transport> (default: \"%s\")\n";
    fprintf(stderr,
            usage_string,
            program,
            DEFAULT_DOWNLOAD_TRANSPORT,
            DEFAULT_MAX_INSTRUCTIONS,
            DEFAULT_LUASRC_DIR,
            DEFAULT_MAX_MEMORY,
            DEFAULT_RESULTS_DIR,
            DEFAULT_SANDBOX_DIR,
            DEFAULT_UPLOAD_TRANSPORT);
}

int censorscope_options_init(censorscope_options_t *options,
                             int argc,
                             char **argv) {
    options->sandbox_dir = strdup(DEFAULT_SANDBOX_DIR);
    if (!options->sandbox_dir) {
        perror("stdrup");
        return -1;
    }
    options->luasrc_dir = strdup(DEFAULT_LUASRC_DIR);
    if (!options->luasrc_dir) {
        free(options->sandbox_dir);
        perror("stdrup");
        return -1;
    }
    options->results_dir = strdup(DEFAULT_RESULTS_DIR);
    if (!options->results_dir) {
        free(options->luasrc_dir);
        free(options->sandbox_dir);
        perror("stdrup");
        return -1;
    }
    options->max_memory = DEFAULT_MAX_MEMORY;
    options->max_instructions = DEFAULT_MAX_INSTRUCTIONS;
    options->download_transport = strdup(DEFAULT_DOWNLOAD_TRANSPORT);
    if (!options->download_transport) {
        free(options->results_dir);
        free(options->luasrc_dir);
        free(options->sandbox_dir);
        perror("stdrup");
        return -1;
    }
    options->upload_transport = strdup(DEFAULT_UPLOAD_TRANSPORT);
    if (!options->upload_transport) {
        free(options->download_transport);
        free(options->results_dir);
        free(options->luasrc_dir);
        free(options->sandbox_dir);
        perror("stdrup");
        return -1;
    }

    const char *short_options = "d:hi:l:m:r:s:u:";
    const struct option long_options[] = {
        {"download-transport", 1, NULL, 'd'},
        {"help", 0, NULL, 'h'},
        {"max-instructions", 1, NULL, 'i'},
        {"luasrc-dir", 1, NULL, 'l'},
        {"max-memory", 1, NULL, 'm'},
        {"results-dir", 1, NULL, 'r'},
        {"sandbox-dir", 1, NULL, 's'},
        {"upload-transport", 1, NULL, 'u'},
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
            break;
        }

        char *first_invalid;
        switch (c) {
        case 'd':
            free(options->download_transport);
            options->download_transport = strdup(optarg);
            if (!options->download_transport) {
                perror("strdup");
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
                perror("strtol");
                censorscope_options_destroy(options);
                return -1;
            }
            if (first_invalid[0] != '\0') {
                fprintf(stderr, "Invalid instruction count.\n");
                return -1;
            }
            break;

        case 'l':
            free(options->luasrc_dir);
            options->luasrc_dir = strdup(optarg);
            if (!options->luasrc_dir) {
                perror("strdup");
                censorscope_options_destroy(options);
                return -1;
            }
            break;

        case 'm':
            errno = 0;
            options->max_memory = strtol(optarg, &first_invalid, 10);
            if (errno) {
                perror("strtol");
                censorscope_options_destroy(options);
                return -1;
            }
            if (first_invalid[0] != '\0') {
                fprintf(stderr, "Invalid max memory.\n");
                censorscope_options_destroy(options);
                return -1;
            }
            break;

        case 'r':
            free(options->results_dir);
            options->results_dir = strdup(optarg);
            if (!options->results_dir) {
                perror("strdup");
                censorscope_options_destroy(options);
                return -1;
            }
            break;

        case 's':
            free(options->sandbox_dir);
            options->sandbox_dir = strdup(optarg);
            if (!options->sandbox_dir) {
                perror("strdup");
                censorscope_options_destroy(options);
                return -1;
            }
            break;

        case 'u':
            free(options->upload_transport);
            options->upload_transport = strdup(optarg);
            if (!options->upload_transport) {
                perror("strdup");
                censorscope_options_destroy(options);
                return -1;
            }
            break;

        default:
            print_usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

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
