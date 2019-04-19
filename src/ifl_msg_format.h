#ifndef _IFL_MSG_FORMAT_H_
#define _IFL_MSG_FORMAT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ifl_conf_parser.h"

#define IFL_FIELD_NAME_MAX                  256
#define IFL_ELEM_DEFAULT_VAL_DATA_MAX       128
#define IFL_ELEM_DEFAULT_VAL_DATA_STR_MAX   256
#define IFL_ELEM_VAL_TYPE_DATA_MAX          16

#define IFL_MSG_ELEM_FIELD          "Field"
#define IFL_MSG_ELEM_VALUE_TYPE     "ValueType"
#define IFL_MSG_ELEM_DEFAULT_VALUE  "DefaultValue"
#define IFL_MSG_ELEM_TAG_FIELD      "TagField"
#define IFL_MSG_ELEM_LENGTH_FIELD   "LengthField"
#define IFL_MSG_ELEM_VALUE_FIELD    "ValueField"

#define IFL_MSG_FIELD_TYPE_V_STR    "V"
#define IFL_MSG_FIELD_TYPE_LV_STR   "LV"
#define IFL_MSG_FIELD_TYPE_TLV_STR  "TLV"
#define IFL_MSG_FIELD_TYPE_S_STR    "S"
#define IFL_MSG_FIELD_TYPE_A_STR    "A"

#define IFL_MSG_FIELD_TYPE_V        1
#define IFL_MSG_FIELD_TYPE_LV       2
#define IFL_MSG_FIELD_TYPE_TLV      3
#define IFL_MSG_FIELD_TYPE_S        4
#define IFL_MSG_FIELD_TYPE_A        5

#define IFL_MSG_FIELD_ATTR_ID       "id"
#define IFL_MSG_FIELD_ATTR_NAME     "name"
#define IFL_MSG_FIELD_ATTR_TYPE     "type"
#define IFL_MSG_FIELD_ATTR_SIZE     "size"

#define IFL_MSG_FIELD_VAL_TYPE_STR_UINT   "uint"
#define IFL_MSG_FIELD_VAL_TYPE_STR_HEX    "hex"

#define IFL_MSG_FIELD_VAL_TYPE_UINT   1
#define IFL_MSG_FIELD_VAL_TYPE_HEX    2

typedef union ifl_msg_field_default_val_st {
    uint8_t hex[IFL_ELEM_DEFAULT_VAL_DATA_MAX];
    uint32_t u32;
}IFL_MSG_FIELD_DEFAULT_VAL;

typedef struct ifl_msg_field_content_st {
    uint32_t id;
    char name[IFL_FIELD_NAME_MAX];
    uint32_t type; /* V, LV, TLV */
    uint32_t size;
    char default_val_type_str[IFL_ELEM_VAL_TYPE_DATA_MAX];
    uint16_t default_val_type;
    char default_val_str[IFL_ELEM_DEFAULT_VAL_DATA_STR_MAX];
    IFL_MSG_FIELD_DEFAULT_VAL default_val;
    uint16_t default_val_size; /* Updated only for hex */
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
    /* For leaf node this depth holds 0, for non leaf node this depth holds the */
    /* number of hop to reach a leaf node */
    /* Only leaf node contains the actual field of type V, all other node contains */
    /* the field of type LV, TLV or S */
    uint16_t depth;
};

typedef struct ifl_field_stack_st {
    IFL_MSG_FIELD **fields;
    uint16_t idx;
    uint16_t size;
}IFL_FIELD_STACK;

int IFL_MsgElemStart(IFL_MSG_FMT_CREATOR *fmt_creator, const char *el, const char **attr);

int IFL_MsgElemData(IFL_MSG_FMT_CREATOR *fmt_creator, const char *data);

int IFL_MsgElemEnd(IFL_MSG_FMT_CREATOR *fmt_creator, const char *el);

void IFL_LogMsgFormat(IFL_MSG_FIELD *msg_format, uint8_t log_level);

void IFL_FreeMsgFormat(IFL_MSG_FIELD *msg_format);

IFL_FIELD_STACK *IFL_InitFieldStack(IFL_MSG_FIELD *msg);

void IFL_FiniFieldStack(IFL_FIELD_STACK *stack);

IFL_MSG_FIELD *IFL_GetNextField(IFL_MSG_FIELD *msg, IFL_FIELD_STACK *stack);

IFL_MSG_FIELD *IFL_GetNthChild(IFL_MSG_FIELD *parent, uint16_t child_num);

IFL_MSG_FIELD *IFL_GetLengthField(IFL_MSG_FIELD *cur);

#ifdef __cplusplus
}
#endif

#endif
