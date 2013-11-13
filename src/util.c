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
    const char *pattern = "sandbox/%s.lua";
    const size_t filename_len = snprintf(NULL, 0, pattern, module);
    char *filename = malloc(filename_len + 1);
    if (!filename) {
        return NULL;
    }
    if (snprintf(filename, filename_len, pattern, module) != filename_len) {
        return NULL;
    }
    return filename;
}
