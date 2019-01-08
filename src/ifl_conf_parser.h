#ifndef _IFL_CONF_PARSER_H_
#define _IFL_CONF_PARSER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ifl_types.h"

typedef struct parser_app_data_st {
    IFL_MSG_FIELD *head;
    IFL_MSG_FIELD *cur;
    uint16_t depth;
    char *element_data;
    uint16_t element_data_size;
    uint8_t err_occured;
}IFL_MSG_FMT_CREATOR;

IFL_MSG_FIELD *IFL_ParseConf(const char *xml_file_name, const char *xml_content);

#ifdef __cplusplus
}
#endif

#endif
