#ifndef _IFL_H_
#define _IFL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Log Level */
#define IFL_LOG_ERR     1
#define IFL_LOG_INFO    2
#define IFL_LOG_DBG     3
#define IFL_LOG_TRACE   4

/* Data type should be void pointer */
#define IFL_CMD_SET_APP_DATA                    1

/* Data type should be void double pointer */
#define IFL_CMD_GET_APP_DATA                    2

typedef struct ifl_st IFL;

typedef void (*IFL_LOG_CB)(uint8_t log_level, const char *log_msg);

IFL *IFL_Init(const char *xml_file_name, const char *xml_content);

void IFL_Fini(IFL *ifl);

uint8_t *IFL_GetFuzzedMsg(IFL *ifl, uint16_t *out_len);

void IFL_FreeFuzzedMsg(uint8_t *ifl_msg);

int IFL_Ctrl(IFL *ifl, uint32_t cmd, void *data, uint16_t data_len);

void IFL_SetLogCB(IFL_LOG_CB log_cb);

#ifdef __cplusplus
}
#endif

#endif
