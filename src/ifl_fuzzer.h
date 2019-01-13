#ifndef _IFL_FUZZER_H_
#define _IFL_FUZZER_H_

#ifdef __cplusplus
extern "C" {
#endif

int IFL_CraftFuzzedMsg(IFL *ifl, uint8_t **out, uint32_t *out_len);

#ifdef __cplusplus
}
#endif

#endif
