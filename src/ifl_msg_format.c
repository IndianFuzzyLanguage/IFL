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
                ERR("Insufficient memory for Name Attr len=%zu\n", strlen(attr_val));
                return -1;
            }
            strcpy(field->field.name, attr_val);
        } else if (!strcmp(attr_name, FIELD_ATTR_TYPE)) {
            if (IFL_ParseFieldAttrType(field, attr_val)) {
                ERR("Unsupported Type Attr val=%s\n", attr_val);
                return -1;
            }
        } else if (!strcmp(attr_name, FIELD_ATTR_SIZE)) {
            if (atoi(attr_val) < 0) {
                ERR("Received invalid field size=%d\n", atoi(attr_val));
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
    IFL_MSG_FIELD *child;

    if (!el) {
        ERR("Element name is NULL\n");
        return -1;
    }
    if (isFieldElement(el)) {
        if (!attr) {
            ERR("Attribute is NULL for FieldElement\n");
            goto err;
        }
        new_field = IFL_AllocMsgField();
        if (!new_field) {
            ERR("New IFL_MSG_FIELD alloc failed for field %s\n", el);
            goto err;
        }
        if (IFL_ParseFieldAttr(new_field, attr)) {
            ERR("Parsing Attr for element=%s failed\n", el);
            goto err;
        }
        if (!fmt_creator->head) {
            fmt_creator->head = new_field;
            fmt_creator->cur = new_field;
        } else if (!fmt_creator->cur) {
            ERR("Receiving new Field Element after completing root tag\n");
            goto err;
        } else if (!fmt_creator->cur->tree.child) {
            fmt_creator->cur->tree.child = new_field;
            new_field->tree.parent = fmt_creator->cur;
            fmt_creator->cur = new_field;
            new_field->list.previous = new_field->list.next = new_field;
        } else {
            child = fmt_creator->cur->tree.child;
            new_field->list.next = child;
            new_field->list.previous = child->list.previous;
            child->list.previous->list.next = new_field;
            child->list.previous = new_field;
        }
        new_field = NULL;
    }

    return 0;
err:
    IFL_FreeMsgField(new_field);
    return -1;
}


int IFL_MsgFieldEnd(IFL_MSG_FMT_CREATOR *fmt_creator, const char *el)
{
    if (isFieldElement(el)) {
        if (fmt_creator->cur) {
            fmt_creator->cur = fmt_creator->cur->tree.parent;
        } else {
            ERR("Abnormal state, Cur=%p, element=%s\n", fmt_creator->cur, el);
            return -1;
        }
    }
    
    return 0;
}
