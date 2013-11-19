#ifndef CENSORSCOPE_UTIL_H
#define CENSORSCOPE_UTIL_H

/* Like sprintf, but automatically allocate enough memory to store the resulting
 * string.
 *
 * Arguments: like printf.
 * Returns: NULL on error, or a formatted string which was allocated with
 * malloc. You must free this string after use.
 *
 */
char *sprintf_malloc(const char *format, ...);

/* Test whether a module name is acceptable for import by modules running in the
 * sandbox. Returns 1 if the name is valid and 0 if not.
 *
 */
int is_valid_module_name(const char *name);

/* Format a module filename.
 *
 * Arguments:
 * - sandbox_dir is the path to the root of the Lua sandbox.
 * - module is the name of the module, without directory or file extension.
 * Returns: the path of the module allocated with malloc. You must free this
 * string.
 *
 */
char *module_filename(const char *sandbox_dir, const char *module);

#endif
