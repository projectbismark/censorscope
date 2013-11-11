#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int is_valid_module_name(const char *name) {
    if (strchr(name, '/')) {
        return 0;
    }
    return 1;
}

char *module_filename(const char *module) {
    if (!is_valid_module_name(module)) {
        return NULL;
    }
    const size_t filename_len = snprintf(NULL, 0,
                                         "sandbox/%s.lua", module) + 1;
    char *filename = malloc(filename_len);
    snprintf(filename, filename_len, "sandbox/%s.lua", module);

    return filename;
}
