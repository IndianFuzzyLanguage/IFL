#ifndef _IFL_FUZZER_H_
#define _IFL_FUZZER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ifl.h"
#include "ifl_buf.h"

typedef enum ifl_fuzz_type_en {
    IFL_FUZZ_TYPE_DEFAULT_VAL_AND_ZERO = 0,
    IFL_FUZZ_TYPE_DEFAULT_VAL_AND_RAND,
    IFL_FUZZ_TYPE_SAMPLE_BASED,
    IFL_FUZZ_TYPE_MAX
}IFL_FUZZ_TYPE;

typedef struct ifl_fuzzer_sample_state_st {
    IFL_BUF *created_msg;
    uint32_t lfield_count;
    uint32_t fuzzed_lfield;
    /* Various types of fuzzing based on sample */
    /* 1. Send the received sample msg as it is */
    uint32_t send_sample_msg:1;
    /* 2. Recreate the msg based on sample and config */
    /* Later this recreated msg is kept as constant and used for all below mentioned fuzzing */
    uint32_t send_recreated_msg:1;
    /* 3. Change each len field one by one and send the fuzzed msg, without adjusting length */
    uint32_t fuzz_len_field_alone:1;
    /* 4. change each len field one by one and also adjust the pay of the corresponding field */
    uint32_t fuzz_len_field:1;
}IFL_FUZZER_SAMPLE_MODE_STATE;

typedef struct ifl_fuzzer_state_st {
    uint32_t fuzzed_id;
    uint32_t fuzzer_type;
    IFL_FUZZER_SAMPLE_MODE_STATE sample_mode_state;
    uint32_t cur_mode_fuzz_finished:1;
    uint32_t fuzz_finished:1;
}IFL_FUZZER_STATE;

typedef int (*IFL_FUZZ_GENERATOR)(IFL *ifl, IFL_BUF *ibuf);
typedef struct ifl_fuzz_type_handler_st {
    const char *type_str;
    IFL_FUZZ_TYPE fuzz_type;
    IFL_FUZZ_GENERATOR fuzz_generator;
}IFL_FUZZ_TYPE_HANDLER;

int IFL_FuzzGenDefaultValAndZero(IFL *ifl, IFL_BUF *ibuf);

int IFL_FuzzGenDefaultValAndRand(IFL *ifl, IFL_BUF *ibuf);

int IFL_FuzzSampleBased(IFL *ifl, IFL_BUF *buf);

int IFL_CraftFuzzedMsg(IFL *ifl, uint8_t **out, uint32_t *out_len);

#ifdef __cplusplus
}
#endif

#endif
