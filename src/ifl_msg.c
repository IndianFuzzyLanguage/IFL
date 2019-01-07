#include <string.h>

#include "ifl_types.h"
#include "ifl_msg.h"
#include "ifl_log.h"
#include "ifl_util.h"
#include "ifl_msg_format.h"

#define IFL_INIT_BUF_SIZE 256
#define IFL_MAX_BUF_SIZE 0x00FFFFFFUL

typedef struct ifl_buf_st {
    uint8_t *buf;
    uint32_t buf_size;      /* Total Buffer size */
    uint32_t data_len;      /* Data filled size */
}IFL_BUF;

int IFL_InitBuf(IFL_BUF *ibuf)
{
    if (ibuf) {
        memset(ibuf, 0, sizeof(IFL_BUF));
        ibuf->buf = calloc(1, IFL_INIT_BUF_SIZE);
        if (!ibuf->buf) {
            ERR("IFL buf init failed");
            goto err;
        }
        ibuf->buf_size = IFL_INIT_BUF_SIZE;
        return 0;
    }
err:
    return -1;
}

int IFL_ResizeBuf(IFL_BUF *ibuf, uint32_t additional_size)
{
    uint8_t *new_buf;
    additional_size = (additional_size < IFL_MAX_BUF_SIZE) ?
                            IFL_MAX_BUF_SIZE : additional_size;
    if ((ibuf) && (ibuf->buf)) {
        if ((ibuf->buf_size + additional_size) >= IFL_MAX_BUF_SIZE) {
            ERR("Reached max IFL buf size %u", ibuf->buf_size + additional_size);
            goto err;
        }
        new_buf = calloc(1, ibuf->buf_size + additional_size);
        if (!new_buf) {
            ERR("IFL resize for %d failed\n", ibuf->buf_size + additional_size);
            goto err;
        }
        memcpy(new_buf, ibuf->buf, ibuf->buf_size);
        free(ibuf->buf);
        ibuf->buf = new_buf;
        ibuf->buf_size += additional_size;
        new_buf = NULL;
        return 0;
    }
err:
    return -1;
}

int IFL_UpdateBuf(IFL_BUF *ibuf, uint8_t *data, uint32_t data_len)
{
    if ((ibuf->buf_size - ibuf->data_len) < data_len) {
        if (IFL_ResizeBuf(ibuf, data_len)) {
            ERR("IFL buf Resize failed\n");
            return -1;
        }
    }
    memcpy(ibuf->buf + ibuf->data_len, data, data_len);
    ibuf->data_len += data_len;
    return 0;
}

void IFL_FiniBuf(IFL_BUF *ibuf)
{
    if ((ibuf) && (ibuf->buf)) {
        free(ibuf->buf);
        memset(ibuf, 0, sizeof(IFL_BUF));
    }
}

int IFL_CraftFuzzedMsg(IFL *ifl, uint8_t **out, uint32_t *out_len)
{
    IFL_MSG_FIELD *cur;
    IFL_BUF ibuf = {0};
    IFL_FIELD_STACK *stack;

    *out = NULL;
    *out_len = 0;
    stack = IFL_InitFieldStack(ifl->msg_format);
    if (!stack) {
        ERR("Field stack init failed");
        return -1;
    }
    if (IFL_InitBuf(&ibuf)) {
        ERR("Initing IFL Buf Failed\n");
        goto err;
    }
    while ((cur = IFL_GetNextField(ifl->msg_format, stack))) {
        if (cur->depth) {
            TRACE("Ignore non leaf field=%s id=%d\n", cur->field.name, cur->field.id);
        } else {
            /* TODO Need to encode in network order */
            TRACE("Updated leaf field=%s id=%d\n", cur->field.name, cur->field.id);
            IFL_UpdateBuf(&ibuf, cur->field.default_value, cur->field.size);
        }
    }
    *out = ibuf.buf;
    *out_len = ibuf.data_len;
    memset(&ibuf, 0, sizeof(ibuf));
    IFL_FiniFieldStack(stack);
    return 0;
err:
    IFL_FiniFieldStack(stack);
    IFL_FiniBuf(&ibuf);
    return -1;
}
