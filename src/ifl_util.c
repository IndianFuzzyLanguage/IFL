#include <string.h>

#include "ifl_types.h"
#include "ifl_util.h"
#include "ifl_log.h"

#define IFL_NthH2N(buf, buf_size, value, idx) \
    buf[((buf_size) - 1) - (idx)] = (((value) >> ((idx) * 8)) & 0xFF)

void IFL_Host2Network(uint8_t *buf, uint32_t buf_size, uint32_t value)
{
    int i;
    TRACE("Updating value=%u on %p", value, buf);
    if (buf_size > sizeof(value)) {
        ERR("Buf size =%u greater than max=%zu", buf_size, sizeof(value));
        return;
    }
    for (i = 0; i < buf_size; i++) {
        IFL_NthH2N(buf, buf_size, value, i);
        TRACE("At idx=%d %u", i, buf[i]);
    }
}

void IFL_GenRandBytes(uint8_t *out, uint32_t size)
{
    int i;
    int rand_val;
    uint32_t size_to_copy;
    srand(size);
    for (i = 0; i < size; i += sizeof(int)) {
        rand_val = rand();
        if ((size - i) < sizeof(int)) {
            size_to_copy = (size - i);
        } else {
            size_to_copy = sizeof(int);
        }
        memcpy(out + i, (uint8_t *)&rand_val, size_to_copy);
    }
}
