#include <stdarg.h>

#include "ifl_types.h"
#include "ifl_log.h"

char *g_log_level_str[IFL_LOG_LEVEL_MAX] = {"", "ERR", "INFO", "DBG", "TRACE"};

IFL_LOG_CB g_log_cb = NULL;

#define IFL_LOG_MAX_SIZE 512
void IFL_LogFunc(uint8_t log_level, const char *format, ...)
{
    char log_msg[IFL_LOG_MAX_SIZE] = {0};
    va_list vargs;
    int ret;

    if (g_log_cb) {
        va_start(vargs, format);
        ret = vsnprintf(log_msg, sizeof(log_msg) - 1, format, vargs);
        va_end(vargs);
        if ((ret > 0) && (ret < (sizeof(log_msg) - 1))) {
            g_log_cb(log_level, log_msg);
        } else {
            g_log_cb(IFL_LOG_ERR, "[ERR] Insufficient log buffer\n");
        }
    }
}

void IFL_SetLogCB(IFL_LOG_CB log_cb)
{
    g_log_cb = log_cb;
}
