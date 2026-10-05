#ifndef NETFORGE_LOG_H
#define NETFORGE_LOG_H

typedef enum { NF_LOG_ERROR, NF_LOG_WARN, NF_LOG_INFO, NF_LOG_DEBUG } nf_log_level_t;
void nf_log_set_level(nf_log_level_t level);
void nf_log(nf_log_level_t level, const char *fmt, ...);

#endif
