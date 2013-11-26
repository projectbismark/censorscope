#include "logging.h"

#include <stdarg.h>
#include <stdio.h>
#include <syslog.h>
#include <time.h>

#ifndef LOGGING_IDENT
#define LOGGING_IDENT "censorscope"
#endif

void logging_init() {
    openlog(LOGGING_IDENT, LOG_CONS, LOG_USER);
}

void logging_destroy() {
    closelog();
}

static void common_log(int level, const char *format, va_list args) {
    va_list args_copy;
    va_copy(args_copy, args);

    vsyslog(level, format, args);

    char buf[20];
    time_t now = time(0);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    fprintf(stderr, "[%s] ", buf);
    switch(level) {
    case LOG_INFO:
        fprintf(stderr, "(INFO) ");
        break;
    case LOG_DEBUG:
        fprintf(stderr, "(DEBUG) ");
        break;
    case LOG_ERR:
        fprintf(stderr, "(ERROR) ");
        break;
    }
    vfprintf(stderr, format, args_copy);
    fprintf(stderr, "\n");
}

void log_debug(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    common_log(LOG_DEBUG, format, ap);
    va_end(ap);
}

void log_info(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    common_log(LOG_INFO, format, ap);
    va_end(ap);
}

void log_error(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    common_log(LOG_ERR, format, ap);
    va_end(ap);
}
