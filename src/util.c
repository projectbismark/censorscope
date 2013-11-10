#include "util.h"

#include <stdio.h>
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
    char filename[255];
    snprintf(filename, sizeof(filename), "sandbox/%s.lua", module);
    return strdup(filename);
}
