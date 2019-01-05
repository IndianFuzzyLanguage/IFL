#ifndef _IFL_LOG_H_
#define _IFL_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define IFL_LOG_LEVEL_MAX 5

extern char *g_log_level_str[IFL_LOG_LEVEL_MAX];

void IFL_LogFunc(uint8_t log_level, const char *format, ...);

#define LOG_FUNC(log_level, file, line_num, format, ...) \
    IFL_LogFunc((uint8_t)log_level, "[%s:%d][%s] "format"\n", file, line_num, \
        g_log_level_str[log_level], ##__VA_ARGS__)

#define LOG(log_level, format, ...) LOG_FUNC(log_level, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define ERR(format, ...) LOG(IFL_LOG_ERR, format, ##__VA_ARGS__)
#define INFO(format, ...) LOG(IFL_LOG_INFO, format, ##__VA_ARGS__)
#define DBG(format, ...) LOG(IFL_LOG_DBG, format, ##__VA_ARGS__)
#define TRACE(format, ...) LOG(IFL_LOG_TRACE, format, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif
