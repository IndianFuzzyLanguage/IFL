#ifndef _IFL_BUF_H_
#define _IFL_BUF_H_

#ifdef __cplusplus
extern "C" {
#endif

#define IFL_INIT_BUF_SIZE 256

#define IFL_MIN_BUF_RESIZE 128

#define IFL_MAX_BUF_SIZE 0x0FFFFFFFUL

typedef struct ifl_buf_st {
    uint8_t *buf;
    uint32_t buf_size;      /* Total Buffer size */
    uint32_t data_len;      /* Data filled size */
}IFL_BUF;

int IFL_InitBuf(IFL_BUF *ibuf);

int IFL_ResizeBuf(IFL_BUF *ibuf, uint32_t additional_size);

int IFL_UpdateBuf(IFL_BUF *ibuf, uint8_t *data, uint32_t data_len);

uint8_t *IFL_GetOffsettedBufPos(IFL_BUF *ibuf);

uint8_t *IFL_SeekBuf(IFL_BUF *ibuf, int off);

void IFL_FiniBuf(IFL_BUF *ibuf);

void IFL_PrintBuf(IFL_BUF *ibuf, const char *name, uint8_t log_level);

#ifdef __cplusplus
}
#endif

#endif
