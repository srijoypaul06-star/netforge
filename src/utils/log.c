#define _POSIX_C_SOURCE 200809L
#include "netforge/log.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <pthread.h>

static nf_log_level_t g_level = NF_LOG_INFO;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

void nf_log_set_level(nf_log_level_t level) { g_level = level; }

void nf_log(nf_log_level_t level, const char *fmt, ...) {
    if (level > g_level) return;
    static const char *names[] = {"ERROR", "WARN", "INFO", "DEBUG"};
    time_t now = time(NULL);
    struct tm tmv;
    localtime_r(&now, &tmv);
    pthread_mutex_lock(&g_lock);
    fprintf(stderr, "%02d:%02d:%02d [%s] ", tmv.tm_hour, tmv.tm_min, tmv.tm_sec, names[level]);
    va_list ap; va_start(ap, fmt); vfprintf(stderr, fmt, ap); va_end(ap);
    fputc('\n', stderr);
    pthread_mutex_unlock(&g_lock);
}
