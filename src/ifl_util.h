#ifndef _IFL_UTIL_H_
#define _IFL_UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#define IFL_CHK_ERR(cond, err_str, stmt) \
    do { \
        if (cond) { \
            ERR(err_str); \
            stmt; \
        } \
    } while(0)

#ifdef __cplusplus
}
#endif

#endif
