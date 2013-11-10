#ifndef CENSORSCOPE_UTIL_H
#define CENSORSCOPE_UTIL_H

/* Test whether a module name is acceptable for import by modules running in the
 * sandbox. Returns 1 if the name is valid and 0 if not.
 *
 */
int is_valid_module_name(const char *name);

char *module_filename(const char *module);

#endif
