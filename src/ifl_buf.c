#include <string.h>

#include "ifl_types.h"
#include "ifl_fuzzer.h"
#include "ifl_log.h"
#include "ifl_util.h"
#include "ifl_buf.h"

/* @Description: Creates a buffer of default size and updates IFL_BUF with length.
 * This does not allocates memory for structure IFL_BUF.
 *
 * @Return: Returns 0 in case of success or else -1
 */
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
        TRACE("Inited IFL_BUF=%p", ibuf->buf);
        return 0;
    }
err:
    return -1;
}

/* @Description: Increases the buffer for the requested size. If the requested size
 * is lesser than minimum size then it increases for the minimum size
 *
 * @Return: Returns 0 in case of success or else -1
 */
int IFL_ResizeBuf(IFL_BUF *ibuf, uint32_t additional_size)
{
    uint8_t *new_buf;
    if ((ibuf) && (ibuf->buf)) {
        additional_size = (additional_size < IFL_MIN_BUF_RESIZE) ?
                                IFL_MIN_BUF_RESIZE : additional_size;
        additional_size = (additional_size > IFL_MAX_BUF_SIZE) ?
                                IFL_MAX_BUF_SIZE : additional_size;
        if ((ibuf->buf_size + additional_size) >= IFL_MAX_BUF_SIZE) {
            ERR("Reached max IFL buf size %u", ibuf->buf_size + additional_size);
            goto err;
        }
        new_buf = calloc(1, ibuf->buf_size + additional_size);
        if (!new_buf) {
            ERR("IFL resize for %d failed", ibuf->buf_size + additional_size);
            goto err;
        }
        memcpy(new_buf, ibuf->buf, ibuf->buf_size);
        free(ibuf->buf);
        ibuf->buf = new_buf;
        ibuf->buf_size += additional_size;
        new_buf = NULL;
        TRACE("Resized IFL_BUF=%p to size=%u", ibuf->buf, ibuf->buf_size);
        return 0;
    }
err:
    return -1;
}

/* @Description: Updates the buffer with passed buffer and increases the data length.
 * If available size is not sufficient resizes it. If NULL buffer is passed then increments
 * only the length in IFL_BUF.
 *
 * @Return: Returns 0 in case of success or else -1
 */
int IFL_UpdateBuf(IFL_BUF *ibuf, uint8_t *data, uint32_t data_len)
{
    if ((ibuf->buf_size - ibuf->data_len) < data_len) {
        if (IFL_ResizeBuf(ibuf, data_len - (ibuf->buf_size - ibuf->data_len))) {
            ERR("IFL buf Resize failed");
            return -1;
        }
    }
    if (data) {
        TRACE("Updating IFL_BUF=%p at index=%d buf_idx=%p", ibuf->buf, ibuf->data_len,
               ibuf->buf + ibuf->data_len);
        memcpy(ibuf->buf + ibuf->data_len, data, data_len);
    }
    ibuf->data_len += data_len;
    return 0;
}

/* @Description: Get buffer pointer to the next byte in which update requires
 *
 * @Return: Returns valid buffer pointer
 */
uint8_t *IFL_GetOffsettedBufPos(IFL_BUF *ibuf)
{
    return (ibuf->buf + ibuf->data_len);
}

/* @Description: Seek buffer forward or backward and get buffer pointer
 *
 * @Return: Valid buffer pointer if seek is of valid offset
 *          or else NULL pointer
 */
uint8_t *IFL_SeekBuf(IFL_BUF *ibuf, int off)
{
    uint8_t *buf;
    uint32_t off_to_seek;

    buf = IFL_GetOffsettedBufPos(ibuf);
    if (off == 0) {
        return buf;
    }
    off_to_seek = (off < 0) ? -off : off;
    if (off < 0) {
        if (ibuf->data_len < off_to_seek) {
            ERR("Invalid seek=%d for available len=%u", off, ibuf->data_len);
            return NULL;
        }
        return (buf - off_to_seek);
    } else {
        if ((ibuf->buf_size - ibuf->data_len) < off_to_seek) {
            ERR("Invalid seek=%d for available len=%u and size=%u", off,
                    ibuf->data_len, ibuf->buf_size);
            return NULL;
        }
        return (buf + off_to_seek);
    }
}

/* @Description: Releases the buffer. Not the IFL_BUF structure memory.
 *
 * @Return: void
 */
void IFL_FiniBuf(IFL_BUF *ibuf)
{
    if ((ibuf) && (ibuf->buf)) {
        free(ibuf->buf);
        memset(ibuf, 0, sizeof(IFL_BUF));
    }
}

