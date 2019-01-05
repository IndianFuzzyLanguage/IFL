#include "ifl_types.h"
#include "ifl_log.h"
#include "ifl_conf_parser.h"
#include "ifl_msg_format.h"
#include "ifl_util.h"

IFL *IFL_Init(const char *xml_file_name, const char *xml_content)
{
    IFL *ifl;

    ifl = calloc(1, sizeof(IFL));
    IFL_CHK_ERR((!ifl), "Mem alloc failed", goto err);
    /*if (!ifl) {
        ERR("Mem alloc failed\n");
        goto err;
    }*/
    if (!(ifl->msg_format = IFL_ParseConf(xml_file_name, xml_content))) {
        ERR("Parse Conf Failed\n");
        goto err;
    }
    DBG("IFL init succeeded\n");
    return ifl;
err:
    IFL_Fini(ifl);
    return NULL;
}

void IFL_Fini(IFL *ifl)
{
    if (ifl) {
        IFL_FreeMsgFormat(ifl->msg_format);
        free(ifl);
    }
}

IFL_MSG *IFL_GetFuzzedMsg(IFL *ifl)
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

void IFL_FreeFuzzedMsg(IFL_MSG *ifl_msg)
{
    if (ifl_msg) {
        free(ifl_msg);
    }
}

int IFL_Ctrl(IFL *ifl, uint32_t cmd, void *data, uint16_t data_len)
{
    if (!ifl) {
        return -1;
    }
    return 0;
}


