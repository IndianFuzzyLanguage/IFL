#include "ifl_types.h"
#include "ifl_log.h"
#include "ifl_conf_parser.h"
#include "ifl_msg_format.h"
#include "ifl_msg.h"
#include "ifl_util.h"

IFL *IFL_Init(const char *xml_file_name, const char *xml_content)
{
    IFL *ifl;

    ifl = calloc(1, sizeof(IFL));
    IFL_CHK_ERR((!ifl), "Mem alloc failed", goto err);

    ifl->msg_format = IFL_ParseConf(xml_file_name, xml_content);
    IFL_CHK_ERR((!ifl->msg_format), "Parse Conf Failed", goto err);

    DBG("IFL=%p init succeeded", ifl);
    IFL_LogMsgFormat(ifl->msg_format, IFL_LOG_DBG);
    return ifl;
err:
    ERR("IFL Init failed");
    IFL_Fini(ifl);
    return NULL;
}

void IFL_Fini(IFL *ifl)
{
    IFL_CHK_ERR((!ifl), "Null pointer passed", return);
    DBG("IFL=%p fini", ifl);
    IFL_FreeMsgFormat(ifl->msg_format);
    free(ifl);
}

int IFL_GetFuzzedMsg(IFL *ifl, uint8_t **out, uint32_t *out_len)
{
    return IFL_CraftFuzzedMsg(ifl, out, out_len);
}

void IFL_FreeFuzzedMsg(uint8_t *ifl_msg)
{
    IFL_CHK_ERR((!ifl_msg), "Null pointer passed", return);
    free(ifl_msg);
}

int IFL_Ctrl(IFL *ifl, uint32_t cmd, void *data, uint16_t data_len)
{
    IFL_CHK_ERR((!ifl), "Null pointer passed", return -1);
    return 0;
}


