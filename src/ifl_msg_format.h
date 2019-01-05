#ifndef _IFL_MSG_FORMAT_H_
#define _IFL_MSG_FORMAT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ifl_conf_parser.h"

#define IFL_FIELD_NAME_MAX 256
#define IFL_FIELD_DEFAULT_VAL_MAX 256

#define IFL_FIELD_TYPE_V    1
#define IFL_FIELD_TYPE_LV   2
#define IFL_FIELD_TYPE_TLV  3
#define IFL_FIELD_TYPE_S    4

typedef struct ifl_msg_field_content_st {
    uint32_t id;
    char name[IFL_FIELD_NAME_MAX];
    uint32_t type; /* V, LV, TLV */
    uint32_t size;
    uint8_t default_value[IFL_FIELD_DEFAULT_VAL_MAX];
}IFL_MSG_FIELD_CONTENT;

typedef struct ifl_tree_state_st {
    IFL_MSG_FIELD *parent;
    IFL_MSG_FIELD *child;
}IFL_TREE_STATE;

typedef struct ifl_list_state_st {
    IFL_MSG_FIELD *previous;
    IFL_MSG_FIELD *next;
}IFL_LIST_STATE;

struct ifl_msg_field_st {
    IFL_TREE_STATE tree;
    IFL_LIST_STATE list;
    IFL_MSG_FIELD_CONTENT field;
    uint16_t depth;
};

int IFL_MsgFieldStart(IFL_MSG_FMT_CREATOR *fmt_creator, const char *el, const char **attr);

int IFL_MsgFieldEnd(IFL_MSG_FMT_CREATOR *fmt_creator, const char *el);

void IFL_LogMsgFormat(IFL_MSG_FIELD *msg_format, uint8_t log_level);

void IFL_FreeMsgFormat(IFL_MSG_FIELD *msg_format);

#ifdef __cplusplus
}
#endif

#endif
