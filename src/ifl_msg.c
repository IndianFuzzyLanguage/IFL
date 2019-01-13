#include <string.h>

#include "ifl_types.h"
#include "ifl_msg.h"
#include "ifl_log.h"
#include "ifl_util.h"
#include "ifl_buf.h"
#include "ifl_msg_format.h"

/* @Description: This function indicates which all field requires size update based on 
 * child. If its a LV or TLV or S then field size needs update based on child size.
 *
 * @Return: Returns 1 for LV, TLV and S field. For others 0.
 */
int IFL_IsFieldSizeUpdateRequired(IFL_MSG_FIELD *field)
{
    switch (field->field.type) {
        case IFL_MSG_FIELD_TYPE_LV:
            return 1;
        case IFL_MSG_FIELD_TYPE_TLV:
            return 1;
        case IFL_MSG_FIELD_TYPE_S:
            return 1;
        default:
            return 0;
    }
    return 0;
}

/* @Description: This function performs some Post Update to field after crafting msg.
 * This function adds child size to all the parents. This performs to parent of type
 * LV, TLV and S. For others no size update is required on parent based on child.
 *
 * @Return: Void
 */
void IFL_FieldPostUpdate(IFL_MSG_FIELD *cur)
{
    IFL_MSG_FIELD *parent = cur->tree.parent;
    while (parent) {
        if (IFL_IsFieldSizeUpdateRequired(parent)) {
            parent->field.size += cur->field.size;
            TRACE("Field=%d size=%d updated", parent->field.id, parent->field.size);
        }
        parent = parent->tree.parent;
    }
}

/* @Description: This function performs some pre update to field before start using for
 * crafting msg. For Fields of type LV, TLV and S, the size is resetted to 0.
 *
 * @Return: void
 *
 */
void IFL_FieldPreUpdate(IFL_MSG_FIELD *cur)
{
    if ((cur) && (IFL_IsFieldSizeUpdateRequired(cur))) {
        cur->field.size = 0;
        TRACE("Field=%d size resetted", cur->field.id);
    }
}

/* @Description: This function crafts the fuzzed msg.
 *
 * @Return: Returns 0 incase of success and -1 incase of failure
 *
 */
int IFL_CraftFuzzedMsg(IFL *ifl, uint8_t **out, uint32_t *out_len)
{
    IFL_MSG_FIELD *cur;
    IFL_BUF ibuf = {0};
    IFL_FIELD_STACK *stack;

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
        IFL_FieldPreUpdate(cur);
        if (cur->depth) {
            TRACE("Ignore non leaf field=%s id=%d type=%d", cur->field.name, cur->field.id,
                        cur->field.type);
        } else {
            /* TODO Need to encode in network order */
            TRACE("Updated leaf field=%s id=%d, size=%d, type=%d", cur->field.name, cur->field.id,
                        cur->field.size, cur->field.type);
            if (cur->field.default_val_type == IFL_MSG_FIELD_VAL_TYPE_HEX) {
                IFL_UpdateBuf(&ibuf, cur->field.default_val.hex, cur->field.size);
            } else {
                IFL_UpdateBuf(&ibuf, NULL, cur->field.size);
            }
        }
        IFL_FieldPostUpdate(cur);
    }
    *out = ibuf.buf;
    *out_len = ibuf.data_len;
    memset(&ibuf, 0, sizeof(ibuf));
    IFL_FiniFieldStack(stack);
    IFL_LogMsgFormat(ifl->msg_format, IFL_LOG_TRACE);
    ifl->state.fuzzed_id++;
    ifl->state.flags |= IFL_FUZZ_STATE_FINISHED;
    return 0;
err:
    IFL_FiniFieldStack(stack);
    IFL_FiniBuf(&ibuf);
    return -1;
}
