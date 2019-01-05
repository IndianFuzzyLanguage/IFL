#include <string.h>

#include "ifl_types.h"
#include "ifl_log.h"
#include "ifl_msg_format.h"
#include "ifl_conf_parser.h"

#define FIELD_ELEMENT "Field"
#define TAG_FIELD_ELEMENT "TagField"
#define LENGTH_FIELD_ELEMENT "LengthField"
#define VALUE_FIELD_ELEMENT "ValueField"

#define FIELD_ATTR_ID "id"
#define FIELD_ATTR_NAME "name"
#define FIELD_ATTR_TYPE "type"
#define FIELD_ATTR_SIZE "size"

void IFL_LogMsgFormat(IFL_MSG_FIELD *msg, uint8_t log_level)
{
    IFL_MSG_FIELD *cur = msg;
    while(cur) {
        LOG(log_level, "Cur=%p, id=%d", cur, cur->field.id);
        cur = cur->tree.child;
    }
}

int isFieldElement(const char *el)
{
    return (strcmp(el, FIELD_ELEMENT) ? 0 : 1);
}

IFL_MSG_FIELD *IFL_AllocMsgField()
{
    return (IFL_MSG_FIELD *)calloc(1, sizeof(IFL_MSG_FIELD));
}

void IFL_FreeMsgField(IFL_MSG_FIELD *msg_field)
{
    if (msg_field) {
        free(msg_field);
    }
}

void IFL_FreeMsgFormat(IFL_MSG_FIELD *msg_format)
{
    /* TODO Need to do all node free here */
    IFL_FreeMsgField(msg_format);
}

int IFL_ParseFieldAttrType(IFL_MSG_FIELD *field, const char *type)
{
    /* TODO Pending */
    return 0;
}

int IFL_ParseFieldAttr(IFL_MSG_FIELD *field, const char **attr)
{
    const char *attr_name;
    const char *attr_val;
    int i;
    for (i = 0; attr[i]; i += 2) {
        attr_name = attr[i];
        attr_val = attr[i + 1];
        if (!strcmp(attr_name, FIELD_ATTR_ID)) {
            field->field.id = atoi(attr_val);
        } else if (!strcmp(attr_name, FIELD_ATTR_NAME)) {
            if (strlen(attr_val) >= sizeof(field->field.name)) {
                ERR("Insufficient memory for Name Attr len=%zu", strlen(attr_val));
                return -1;
            }
            strcpy(field->field.name, attr_val);
        } else if (!strcmp(attr_name, FIELD_ATTR_TYPE)) {
            if (IFL_ParseFieldAttrType(field, attr_val)) {
                ERR("Unsupported Type Attr val=%s", attr_val);
                return -1;
            }
        } else if (!strcmp(attr_name, FIELD_ATTR_SIZE)) {
            if (atoi(attr_val) < 0) {
                ERR("Received invalid field size=%d", atoi(attr_val));
                return -1;
            }
            field->field.size = (uint16_t)(atoi(attr_val));
        }
    }
    return 0;
}

int IFL_MsgFieldStart(IFL_MSG_FMT_CREATOR *fmt_creator, const char *el, const char **attr)
{
    IFL_MSG_FIELD *new_field = NULL;
    IFL_MSG_FIELD *first_child;

    if (!el) {
        ERR("Element name is NULL");
        return -1;
    }
    TRACE("Start Element=%s", el);
    if (isFieldElement(el)) {
        if (!attr) {
            ERR("Attribute is NULL for FieldElement");
            goto err;
        }
        new_field = IFL_AllocMsgField();
        if (!new_field) {
            ERR("New IFL_MSG_FIELD alloc failed for field %s", el);
            goto err;
        }
        if (IFL_ParseFieldAttr(new_field, attr)) {
            ERR("Parsing Attr for element=%s failed", el);
            goto err;
        }
        if (!fmt_creator->head) {
            fmt_creator->head = new_field;
            fmt_creator->cur = new_field;
        } else if (!fmt_creator->cur) {
            ERR("Receiving new Field Element after completing root tag");
            goto err;
        } else if (!fmt_creator->cur->tree.child) {
            /* First child */
            /* Update Tree states */
            /* - Update parent node's child */
            /* - Update child node's parent */
            /* - Keep child node as current node */
            fmt_creator->cur->tree.child = new_field;
            new_field->tree.parent = fmt_creator->cur;
            fmt_creator->cur = new_field;
            /* Update List states */
            new_field->list.previous = new_field->list.next = new_field;
        } else {
            /* Not a first child */
            /* Update Tree states */
            /* - No need to update parent node's child */
            /* - Update child node's parent */
            /* - Keep child node as current node */
            new_field->tree.parent = fmt_creator->cur;
            fmt_creator->cur = new_field;
            /* Update List states */
            /* - Get First child and update list states */
            first_child = fmt_creator->cur->tree.parent->tree.child;
            new_field->list.next = first_child;
            new_field->list.previous = first_child->list.previous;
            first_child->list.previous->list.next = new_field;
            first_child->list.previous = new_field;
        }
        new_field = NULL;
    }

    TRACE("Msg Field head=%p, cur=%p", fmt_creator->head, fmt_creator->cur);
    return 0;
err:
    IFL_FreeMsgField(new_field);
    return -1;
}

int IFL_MsgFieldEnd(IFL_MSG_FMT_CREATOR *fmt_creator, const char *el)
{
    TRACE("End Element=%s", el);
    if (isFieldElement(el)) {
        if (fmt_creator->cur) {
            fmt_creator->cur = fmt_creator->cur->tree.parent;
        } else {
            ERR("Abnormal state, Cur=%p, element=%s", fmt_creator->cur, el);
            return -1;
        }
    }
    
    TRACE("Msg Field head=%p, cur=%p", fmt_creator->head, fmt_creator->cur);
    return 0;
}

