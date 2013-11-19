#ifndef CENSORSCOPE_OPTIONS_H
#define CENSORSCOPE_OPTIONS_H

#include <stddef.h>

typedef struct {
    char *sandbox_dir;
    char *luasrc_dir;
    size_t max_memory;
    long max_instructions;
} censorscope_options_t;

int censorscope_options_init(censorscope_options_t *options,
                             int argc,
                             char **argv);

int censorscope_options_destroy(censorscope_options_t *options);

#endif
