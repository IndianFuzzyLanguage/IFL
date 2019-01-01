#ifndef _IFL_TYPES_H_
#define _IFL_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ifl.h"

typedef struct ifl_msg_field_st IFL_MSG_FIELD;

struct ifl_st {
    uint32_t msg_id;
    IFL_MSG_FIELD *msg_format;
};

#ifdef __cplusplus
}
#endif

#endif
