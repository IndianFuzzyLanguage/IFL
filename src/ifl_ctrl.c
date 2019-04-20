#include "ifl_types.h"
#include "ifl_log.h"
#include "ifl_ctrl.h"

int IFL_CtrlOp(IFL *ifl, uint32_t cmd, void *data, uint32_t data_len)
{
    switch(cmd) {
        case IFL_CTRL_CMD_SET_SAMPLE_MSG:
            ifl->sample_msg = data;
            ifl->sample_msg_len = data_len;
            break;
        default:
            ERR("Unknown cmd=%u\n", cmd);
            return -1;
    }
    return 0;
}
