#ifndef _IFL_LOG_H_
#define _IFL_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define IFL_LOG_ERR   1
#define IFL_LOG_INFO  2
#define IFL_LOG_DBG   3

#define LOG_FUNC(log_level, file, line_num, format, ...) \
    printf("[%s:%d][%d] "format, file, line_num, log_level, ##__VA_ARGS__)

#define ERR(format, ...) LOG_FUNC(IFL_LOG_ERR, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define DBG(format, ...) LOG_FUNC(IFL_LOG_DBG, __FILE__, __LINE__, format, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif
