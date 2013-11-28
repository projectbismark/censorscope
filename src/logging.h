#ifndef CENSORSCOPE_LOGGING_H
#define CENSORSCOPE_LOGGING_H

/* Call this function before using log_debug, log_info, or log_error. */
void logging_init();

void logging_destroy();

void log_debug(const char *format, ...);

void log_info(const char *format, ...);

void log_error(const char *format, ...);

#endif
