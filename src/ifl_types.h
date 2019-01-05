#ifndef _IFL_TYPES_H_
#define _IFL_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ifl.h"

#define IFL_MAX_MSG_FIELD_DEPTH     16000

typedef struct ifl_msg_field_st IFL_MSG_FIELD;

typedef struct ifl_fuzzer_state_st {
    uint32_t fuzzed_id;
}IFL_FUZZER_STATE;

struct ifl_st {
    uint32_t msg_id;
    IFL_MSG_FIELD *msg_format;
};

#ifdef __cplusplus
}
#endif

#endif
