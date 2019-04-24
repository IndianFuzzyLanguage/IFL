#include <string.h>

#include "ifl_types.h"
#include "ifl_fuzzer.h"
#include "ifl_log.h"
#include "ifl_util.h"
#include "ifl_buf.h"
#include "ifl_msg_format.h"

IFL_FUZZ_TYPE_HANDLER g_fuzz_generator[IFL_FUZZ_TYPE_MAX] = {
    {"IFL_FUZZ_TYPE_DEFAULT_VAL_AND_ZERO", IFL_FUZZ_TYPE_DEFAULT_VAL_AND_ZERO,
        IFL_FuzzGenDefaultValAndZero},
    {"IFL_FUZZ_TYPE_DEFAULT_VAL_AND_RAND", IFL_FUZZ_TYPE_DEFAULT_VAL_AND_RAND,
        IFL_FuzzGenDefaultValAndRand},
    {"IFL_FUZZ_TYPE_DEFAULT_VAL_AND_RAND", IFL_FUZZ_TYPE_SAMPLE_BASED,
        IFL_FuzzSampleBased}
};

/* @Description: This function indicates which all field requires size update based on 
 * child. If its a LV or TLV or S then field size needs update based on child size.
 *
 * @Return: Returns 1 for LV, TLV and S field. For others 0.
 */
int IFL_IsFieldSizeUpdateRequired(IFL_MSG_FIELD *field)
{
    switch (field->field.type) {
        case IFL_MSG_FIELD_TYPE_V:
            return 0;
        case IFL_MSG_FIELD_TYPE_LV:
            return 1;
        case IFL_MSG_FIELD_TYPE_TLV:
            return 1;
        case IFL_MSG_FIELD_TYPE_S:
            return 1;
        case IFL_MSG_FIELD_TYPE_A:
            return 1;
        default:
            ERR("Unknown type=%d", field->field.type);
            return 0;
    }
    return 0;
}

/* @Description: Updates integer value to pkt buffer in network byte order
 *
 * @Return: Void
 */
void IFL_UpdatePrevLengthField(uint8_t *buf, uint32_t field_size, uint32_t value)
{
    TRACE("Buf=%p updating of size=%d, value=%u", buf, field_size, value);
    IFL_Host2Network(buf, field_size, value);
}

/* @Description: This function performs some Post Update to field after crafting msg.
 * 1) Adds child size to all the parents. This performs to parent of type
 * LV, TLV, S or A. For others no size update is required on parent based on child.
 * 2) If current field's grand parent is of LV or TLV type, then update length in 
 * parent's previous field's (previous sibling) value.
 *
 * @Return: Void
 */
int IFL_FieldPostUpdate(IFL_MSG_FIELD *cur, IFL_BUF *ibuf)
{
    IFL_MSG_FIELD *parent = cur;
    IFL_MSG_FIELD *length_field_of_parent;
    uint8_t *len_field_buf;
    if (cur->field.size == 0) {
        /* If current field is LV, TLV, S or A, length might be zero. */
        /* If current field len is zero, then no need to update parent's and parent */
        /* sibling length. */
        return 0;
    }
    TRACE("Need to update current field=%s size=%d", cur->field.name, cur->field.size);
    while (parent) {
        /* 1) Adds child size to all the parents of type LV, TLV, S and A */
        if (parent != cur && IFL_IsFieldSizeUpdateRequired(parent)) {
            parent->field.size += cur->field.size;
            TRACE("Parent Field=%s updated size=%u", parent->field.name, parent->field.size);
        }
        /* 2) If parent field is "V" field of LV or TLV grand parent, then update length */
        /* in previous field's value of parent */
        length_field_of_parent = IFL_GetLengthField(parent);
        if (length_field_of_parent) {
            if (length_field_of_parent == parent->list.previous) {
                len_field_buf = IFL_SeekBuf(ibuf,
                                -(parent->field.size + length_field_of_parent->field.size));
                if (len_field_buf == NULL) {
                    ERR("Invalid seeked buffer for length field update of field=%s",
                            cur->field.name);
                    return -1;
                }
                IFL_UpdatePrevLengthField(len_field_buf, length_field_of_parent->field.size,
                                          parent->field.size);
                TRACE("Length field=%s of Parent Field=%s, value updated to=%d",
                       length_field_of_parent->field.name, parent->field.name, parent->field.size);
            }
        }
        parent = parent->tree.parent;
        IFL_PrintBuf(ibuf, "PostUpdate", IFL_LOG_TRACE);
    }
    return 0;
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
        /* Reset size of LV, TLV, S and A field to zero */
        /* Later based on child field size, this field gets updated */
        cur->field.size = 0;
        TRACE("Field=%s size resetted", cur->field.name);
    }
}

int IFL_FuzzGenDefaultVal(IFL *ifl, IFL_BUF *ibuf, uint32_t fuzz_type)
{
    IFL_MSG_FIELD *cur;
    IFL_FIELD_STACK *stack;
    uint8_t *rand = NULL;
    int ret_val = -1;

    stack = IFL_InitFieldStack(ifl->msg_format);
    IFL_CHK_ERR((!stack), "Field stack init failed", return -1);
    while ((cur = IFL_GetNextField(ifl->msg_format, stack))) {
        IFL_FieldPreUpdate(cur);
        if (cur->depth) {
            TRACE("Ignore non leaf field=%s id=%d type=%d", cur->field.name, cur->field.id,
                        cur->field.type);
        } else {
            TRACE("Updating leaf field=%s id=%d, size=%d, type=%d", cur->field.name, cur->field.id,
                        cur->field.size, cur->field.type);
            if (cur->field.default_val_type == IFL_MSG_FIELD_VAL_TYPE_HEX) {
                IFL_UpdateBuf(ibuf, cur->field.default_val.hex, cur->field.size);
            } else if (cur->field.default_val_type == IFL_MSG_FIELD_VAL_TYPE_UINT) {
                IFL_Host2Network(IFL_GetOffsettedBufPos(ibuf), cur->field.size,
                                 cur->field.default_val.u32);
                IFL_UpdateBuf(ibuf, NULL, cur->field.size);
            } else {
                if (fuzz_type == IFL_FUZZ_TYPE_DEFAULT_VAL_AND_RAND) {
                    rand = calloc(1, cur->field.size);
                    if (rand) {
                        IFL_GenRandBytes(rand, cur->field.size);
                    }
                }
                IFL_UpdateBuf(ibuf, rand, cur->field.size);
                if (rand) {
                    free(rand);
                    rand = NULL;
                }
            }
        }
        if (IFL_FieldPostUpdate(cur, ibuf)) {
            ERR("Field post update failed");
            goto err;
        }
    }
    ifl->state.cur_mode_fuzz_finished = 1;
    ret_val = 0;
err:
    IFL_FiniFieldStack(stack);
    return ret_val;
}

/* @Description: Generates fuzzed msg with default value and keep zero for others
 *
 * @Return: Returns 0 incase of success or else -1
 */
int IFL_FuzzGenDefaultValAndZero(IFL *ifl, IFL_BUF *ibuf)
{
    return IFL_FuzzGenDefaultVal(ifl, ibuf, IFL_FUZZ_TYPE_DEFAULT_VAL_AND_ZERO);
}

/* @Description: Generates fuzzed msg with default value and keep zero for others
 *
 * @Return: Returns 0 incase of success or else -1
 */
int IFL_FuzzGenDefaultValAndRand(IFL *ifl, IFL_BUF *ibuf)
{
    return IFL_FuzzGenDefaultVal(ifl, ibuf, IFL_FUZZ_TYPE_DEFAULT_VAL_AND_RAND);
}

/* @Description: Generates fuzzed msg based on sample msg
 *
 * @Return: Returns 0 incase of success or else -1
 */
int IFL_CreateMsgBasedOnSample(IFL *ifl, IFL_BUF *ibuf)
{
    IFL_MSG_FIELD *cur;
    IFL_FIELD_STACK *stack;
    uint32_t sample_msg_off = 0;
    uint8_t *data_to_update;

    stack = IFL_InitFieldStack(ifl->msg_format);
    IFL_CHK_ERR((!stack), "Field stack init failed", return -1);
    ifl->state.sample_mode_state.lfield_count = 0;
    while ((cur = IFL_GetNextField(ifl->msg_format, stack))) {
        data_to_update = NULL;
        IFL_FieldPreUpdate(cur);
        if (cur->depth) {
            /* No need to do anything for non leaf node */
            continue;
        }
        if (!IFL_IsFieldTypeL(cur)) {
            TRACE("Updating Non L field=%s", cur->field.name);
            data_to_update = ifl->sample_msg + sample_msg_off;
        } else {
            ifl->state.sample_mode_state.lfield_count++;
        }
        if (IFL_UpdateBuf(ibuf, data_to_update, cur->field.size)) {
            ERR("Update buf failed");
            goto err;
        }
        sample_msg_off += cur->field.size;
        if (IFL_FieldPostUpdate(cur, ibuf)) {
            ERR("Field post update failed");
            goto err;
        }
        IFL_PrintBuf(ibuf, "SampleBasedFuzz", IFL_LOG_TRACE);
    }
    IFL_FiniFieldStack(stack);
    return 0;
err:
    IFL_FiniFieldStack(stack);
    return -1;
}

/* @Description: Generates fuzzed msg based on sample msg gets fuzzed on
 * each L field
 *
 * @Return: Returns 0 incase of success or else -1
 */
int IFL_ModifyCreatedMsgLField(IFL *ifl, IFL_BUF *ibuf)
{
    IFL_MSG_FIELD *cur;
    IFL_FIELD_STACK *stack;
    int ret_val = -1;
    uint32_t lfield_count = 0;
    uint32_t msg_off = 0;
    uint32_t lfield_value;

    if (IFL_UpdateBuf(ibuf, ifl->state.sample_mode_state.created_msg->buf,
                            ifl->state.sample_mode_state.created_msg->data_len)) {
        ERR("Update buf failed");
        return -1;
    }
    stack = IFL_InitFieldStack(ifl->msg_format);
    IFL_CHK_ERR((!stack), "Field stack init failed", return -1);
    while ((cur = IFL_GetNextField(ifl->msg_format, stack))) {
        /* 1. Traverse each node, and start skipping each L field to reach the L field which
         * has the count as fuzzed_lfield. That means fuzzed_lfield gets fuzzed in this
         * this time call to this function.
         * 2. For each node (L and non L) update the msg_off so that we can update the value once
         * the fuzzed_lfield is found.*/
        if (cur->depth) {
            continue;
        }
        if (IFL_IsFieldTypeL(cur)) {
            lfield_count++;
            if (lfield_count == ifl->state.sample_mode_state.fuzzed_lfield) {
                IFL_Network2Host(ibuf->buf + msg_off, cur->field.size, &lfield_value);
                DBG("L field=%s[count=%u] value=%u will be reduced by 1",
                        cur->field.name, lfield_count, lfield_value);
                if (lfield_value) {
                    lfield_value--;
                }
                IFL_Host2Network(ibuf->buf + msg_off, cur->field.size, lfield_value);
                /* Decided changes on L field done */
                ret_val = 0;
                goto end;
            }
        }
        /* field.size gets updated for all fields including LV, TLV, S and A type */
        /* in previous call to IFL_CreateMsgBasedOnSample */
        /* So we can utilize here */
        msg_off += cur->field.size;
    }
end:
    IFL_FiniFieldStack(stack);
    return ret_val;
}

/* @Description: Generates fuzzed msg based on sample msg
 *
 * @Return: Returns 0 incase of success or else -1
 */
int IFL_FuzzSampleBased(IFL *ifl, IFL_BUF *ibuf)
{
    IFL_FUZZER_SAMPLE_MODE_STATE *sample_mode_state;
    sample_mode_state = &ifl->state.sample_mode_state;
    IFL_CHK_ERR((!ifl->sample_msg || !ifl->sample_msg_len), "Sample Msg not available", return -1);
    /* Sample based Fuzzed msg are created on below modes
     * 1. First send sample msg as it is
     * 2. Recreate from sample msg based on config and start sending by modifying each length
     * field */
    if (sample_mode_state->send_sample_msg == 0) {
        /* 1. First send sample msg as it is */
        if (IFL_UpdateBuf(ibuf, ifl->sample_msg, ifl->sample_msg_len)) {
            ERR("Sending sample msg failed");
            goto err;
        }
        sample_mode_state->send_sample_msg = 1;
    } else {
        /* 2. Recreate from sample msg and send by modifying each length field */
        if (sample_mode_state->created_msg == NULL) {
            if (IFL_CreateMsgBasedOnSample(ifl, ibuf)) {
                ERR("Sample Based fuzz failed");
                goto err;
            }
            sample_mode_state->created_msg = IFL_DupBuf(ibuf);
            sample_mode_state->fuzzed_lfield = 0;
            TRACE("Recreated msg from sample, lfield count=%u", sample_mode_state->lfield_count);
        } else {
            /* First time send recreated msg as it is */
            /* Then later start modifying each length field one by one */
            sample_mode_state->fuzzed_lfield++;
            if (IFL_ModifyCreatedMsgLField(ifl, ibuf)) {
                ERR("Modify Created Msg L field failed");
                goto err;
            }
        }
        if ((sample_mode_state->lfield_count == 0)
                || (sample_mode_state->fuzzed_lfield + 1 >= sample_mode_state->lfield_count)) {
            ifl->state.cur_mode_fuzz_finished = 1;
        }
    }
    return 0;
err:
    return -1;
}

/* @Description: Crafts the fuzzed msg.
 *
 * @Return: Returns 0 incase of success and -1 incase of failure
 *
 */
int IFL_CraftFuzzedMsg(IFL *ifl, uint8_t **out, uint32_t *out_len)
{
    IFL_BUF ibuf = {0};

    IFL_CHK_ERR(IFL_InitBuf(&ibuf), "Initing IFL Buf Failed", goto err);
    if (g_fuzz_generator[ifl->state.fuzzer_type].fuzz_generator(ifl, &ibuf)) {
        ERR("Fuzz generator for default value failed");
        goto err;
    }
    *out = ibuf.buf;
    *out_len = ibuf.data_len;
    memset(&ibuf, 0, sizeof(ibuf));
    ifl->state.fuzzed_id++;
    TRACE("Created Fuzzed msg count=%u of type=%s",
            ifl->state.fuzzed_id, g_fuzz_generator[ifl->state.fuzzer_type].type_str);
    if (ifl->state.cur_mode_fuzz_finished) {
        ifl->state.fuzzer_type++;
        ifl->state.cur_mode_fuzz_finished = 0;
    }
    if (ifl->state.fuzzer_type == IFL_FUZZ_TYPE_MAX) {
        ifl->state.fuzz_finished = 1;
    }
    return 0;
err:
    IFL_FiniBuf(&ibuf);
    return -1;
}
