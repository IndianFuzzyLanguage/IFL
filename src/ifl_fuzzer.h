#ifndef _IFL_FUZZER_H_
#define _IFL_FUZZER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ifl.h"
#include "ifl_buf.h"

/* Flags for IFL_FUZZER_STATE */
#define IFL_FUZZ_STATE_FINISHED     0x00000001UL

typedef enum ifl_fuzz_type_en {
    IFL_FUZZ_TYPE_DEFAULT_VAL_AND_ZERO = 1,
    IFL_FUZZ_TYPE_DEFAULT_VAL_AND_RAND,
    IFL_FUZZ_TYPE_MAX
}IFL_FUZZ_TYPE;

typedef struct ifl_fuzzer_state_st {
    uint32_t fuzzed_id;
    uint32_t flags;
    uint32_t fuzzer_type;
}IFL_FUZZER_STATE;

typedef int (*IFL_FUZZ_GENERATOR)(IFL *ifl, IFL_BUF *ibuf);
typedef struct ifl_fuzz_type_handler_st {
    IFL_FUZZ_TYPE fuzz_type;
    IFL_FUZZ_GENERATOR fuzz_generator;
}IFL_FUZZ_TYPE_HANDLER;

int IFL_FuzzGenDefaultValAndZero(IFL *ifl, IFL_BUF *ibuf);

int IFL_FuzzGenDefaultValAndRand(IFL *ifl, IFL_BUF *ibuf);

int IFL_CraftFuzzedMsg(IFL *ifl, uint8_t **out, uint32_t *out_len);

#ifdef __cplusplus
}
#endif

#endif
