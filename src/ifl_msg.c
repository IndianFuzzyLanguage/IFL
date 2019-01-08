#include <string.h>

#include "ifl_types.h"
#include "ifl_msg.h"
#include "ifl_log.h"
#include "ifl_util.h"
#include "ifl_buf.h"
#include "ifl_msg_format.h"

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
        ERR("Initing IFL Buf Failed");
        goto err;
    }
    while ((cur = IFL_GetNextField(ifl->msg_format, stack))) {
        if (cur->depth) {
            TRACE("Ignore non leaf field=%s id=%d", cur->field.name, cur->field.id);
        } else {
            /* TODO Need to encode in network order */
            TRACE("Updated leaf field=%s id=%d", cur->field.name, cur->field.id);
            IFL_UpdateBuf(&ibuf, cur->field.default_val, cur->field.size);
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
