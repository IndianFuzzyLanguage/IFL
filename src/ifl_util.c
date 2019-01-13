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
