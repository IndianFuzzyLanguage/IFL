#ifndef _IFL_CTRL_H_
#define _IFL_CTRL_H_

#ifdef __cplusplus
extern "C" {
#endif


int IFL_CtrlOp(IFL *ifl, uint32_t cmd, void *data, uint32_t data_len);

#ifdef __cplusplus
}
#endif

#endif
