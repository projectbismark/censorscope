#include "util.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *sprintf_malloc(const char *format, ...) {
    va_list ap;

    va_start(ap, format);
    int count = vsnprintf(NULL, 0, format, ap);
    char *output = malloc(count + 1);
    if (!output) {
        perror("malloc");
        va_end(ap);
        return NULL;
    }
    va_end(ap);

    va_start(ap, format);
    if (vsnprintf(output, count + 1, format, ap) != count) {
        perror("vsnprintf");
        free(output);
        va_end(ap);
        return NULL;
    }
    va_end(ap);

    return output;
}

int is_valid_module_name(const char *name) {
    if (strchr(name, '/')) {
        return 0;
    }
    return 1;
}

char *module_filename(const char *sandbox_dir, const char *module) {
    if (!is_valid_module_name(module)) {
        return NULL;
    }
    char *filename = sprintf_malloc("%s/%s.lua", sandbox_dir, module);
    if (!filename) {
        return NULL;
    }
    return filename;
}
