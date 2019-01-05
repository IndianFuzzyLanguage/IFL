#ifndef _IFL_LOG_H_
#define _IFL_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define IFL_LOG_LEVEL_MAX 5

extern char *g_log_level_str[IFL_LOG_LEVEL_MAX];

void IFL_LogFunc(uint8_t log_level, const char *format, ...);

#define LOG_FUNC(log_level, file, line_num, format, ...) \
    IFL_LogFunc((uint8_t)log_level, "[%s:%d][%s] "format, file, line_num, \
        g_log_level_str[log_level], ##__VA_ARGS__)

#define ERR(format, ...) LOG_FUNC(IFL_LOG_ERR, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define INFO(format, ...) LOG_FUNC(IFL_LOG_INFO, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define DBG(format, ...) LOG_FUNC(IFL_LOG_DBG, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define TRACE(format, ...) LOG_FUNC(IFL_LOG_TRACE, __FILE__, __LINE__, format, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif
