#include "ifl_types.h"
#include "ifl_log.h"

IFL *IFL_init(const char *xml_file_name, const char *xml_content)
{
    IFL *ifl;

    ifl = calloc(1, sizeof(IFL));
    if (!ifl) {
        ERR("Mem alloc failed\n");
        goto err;
    }

    DBG("IFL init succeeded\n");
    return ifl;
err:
    IFL_fini(ifl);
    return NULL;
}

void IFL_fini(IFL *ifl)
{
    if (ifl) {
        free(ifl);
    }
}

IFL_MSG *IFL_getFuzzedMsg(IFL *ifl)
{
    IFL_MSG *ifl_msg;

    ifl_msg = calloc(1, sizeof(IFL_MSG));
    if (!ifl) {
        ERR("Mem alloc failed\n");
        goto err;
    }

    DBG("IFL Fuzzed msg created %d\n", ifl_msg->fuzzed_id);
    return ifl_msg;
err:
    free(ifl_msg);
    return NULL;
}

void IFL_freeFuzzedMsg(IFL_MSG *ifl_msg)
{
    if (ifl_msg) {
        free(ifl_msg);
    }
}

IFL *IFL_ctrl(IFL *ifl, uint32_t cmd, void *data, uint16_t data_len)
{
    return 0;
}


