#ifndef _IFL_TYPES_H_
#define _IFL_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ifl.h"
#include "ifl_fuzzer.h"

#define IFL_MAX_MSG_FIELD_DEPTH     16000

typedef struct ifl_msg_field_st IFL_MSG_FIELD;

struct ifl_st {
    uint32_t msg_id;
    IFL_MSG_FIELD *msg_format;
    uint8_t *sample_msg;
    uint32_t sample_msg_len;
    IFL_FUZZER_STATE state;
};

#ifdef __cplusplus
}
#endif

#endif
