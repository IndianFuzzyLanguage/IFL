#include <string.h>

#include "ifl_types.h"
#include "ifl_msg_format.h"
#include "ifl_conf_parser.h"
#include "ifl_util.h"
#include "ifl_log.h"

IFL_FIELD_STACK *IFL_InitFieldStack(IFL_MSG_FIELD *msg)
{
    IFL_FIELD_STACK *stack;
    stack = (IFL_FIELD_STACK *)calloc(1, (sizeof(IFL_FIELD_STACK)
                    + ((msg->depth + 1) * sizeof(IFL_MSG_FIELD *))));
    if (!stack) {
        ERR("Field stack alloc of size %u failed", msg->depth + 1);
        return NULL;
    }
    stack->fields = (IFL_MSG_FIELD **)(((uint8_t *)stack) + sizeof(IFL_FIELD_STACK));
    stack->size = msg->depth + 1;
    return stack;
}

int IFL_PushFieldStack(IFL_FIELD_STACK *stack, IFL_MSG_FIELD *data)
{
    if (stack->idx == stack->size) {
        ERR("Field stack is full");
        return -1;
    }
    stack->fields[stack->idx] = data;
    stack->idx += 1;
    return 0;
}

IFL_MSG_FIELD *IFL_PopFieldStack(IFL_FIELD_STACK *stack)
{
    IFL_MSG_FIELD *ret = NULL;
    if (stack->idx) {
        stack->idx -= 1;
        ret = stack->fields[stack->idx];
    }
    return ret;
}

IFL_MSG_FIELD *IFL_PeekFieldStack(IFL_FIELD_STACK *stack)
{
    IFL_MSG_FIELD *ret = NULL;
    if (stack->idx) {
        ret = stack->fields[stack->idx - 1];
    }
    return ret;
}

void IFL_FiniFieldStack(IFL_FIELD_STACK *stack)
{
    IFL_CHK_ERR((!stack), "Null pointer passed", return);
    free(stack);
}

/* @Description: This function traverses the Tree in Depth First Search and returns each
 * node one by one.
 *
 * @Return: Returns the current selected node. If traversal finishes returns NULL
 *
 */
IFL_MSG_FIELD *IFL_GetNextField(IFL_MSG_FIELD *msg, IFL_FIELD_STACK *stack)
{
    IFL_MSG_FIELD *cur;
    IFL_MSG_FIELD *prev;
    IFL_MSG_FIELD *prev_parent;

    if (!stack->idx) {
        cur = msg;
    } else {
        cur = IFL_PeekFieldStack(stack);
        if (cur->tree.child) {
            cur = cur->tree.child;
        } else {
            cur = NULL;
            do {
                prev = IFL_PopFieldStack(stack);
                prev_parent = prev ? prev->tree.parent : NULL;
                if ((!prev) || (!prev_parent)) {
                    break;
                }
                /* Check if it has some more siblings */
                /* If so keep that as cur or else go back to next parent */
                if (prev->list.next != prev_parent->tree.child) {
                    /* A node's parent's child is the first node on doubly list */
                    /* If current node's next != first node then its new sibling */
                    cur = prev->list.next;
                }
            } while(!cur);
        }
    }
    if (cur) {
        if (IFL_PushFieldStack(stack, cur)) {
            ERR("Field stack push failed");
            return NULL;
        }
    }
    return cur;
}

char *IFL_GetFieldDefaultValStr(IFL_MSG_FIELD *field, char *out, uint16_t out_size)
{
    int ret;
    int i, off = 0;
    switch(field->field.default_val_type) {
        case IFL_MSG_FIELD_VAL_TYPE_UINT:
            snprintf(out, out_size, "%u", field->field.default_val.u32);
            break;
        case IFL_MSG_FIELD_VAL_TYPE_HEX:
            for (i = 0; i < field->field.default_val_size; i++) {
                ret = snprintf(out + off, out_size - off, "%02X", field->field.default_val.hex[i]);
                if ((ret > 0) && (ret < (out_size - off))) {
                    off += ret;
                }
            }
            break;
        default:
            snprintf(out, out_size, "Nothing");
            break;
    }
    return out;
}

void IFL_LogMsgFormat(IFL_MSG_FIELD *msg, uint8_t log_level)
{
    IFL_MSG_FIELD *cur;
    IFL_FIELD_STACK *stack;
    stack = IFL_InitFieldStack(msg);
    char val_str[IFL_ELEM_DEFAULT_VAL_DATA_STR_MAX] = {0};
    LOG(log_level, "Msg tree with max depth=%u", msg->depth);
    while ((stack) && (cur = IFL_GetNextField(msg, stack))) {
        LOG(log_level, "Cur=%p, id=%d, name=%s, size=%d, type=%d, DefaultVal=%s, depth=%d",
                cur, cur->field.id, cur->field.name, cur->field.size, cur->field.type,
                IFL_GetFieldDefaultValStr(cur, val_str, sizeof(val_str)), cur->depth);
    }
    IFL_FiniFieldStack(stack);
}

int isElementField(const char *el)
{
    return (strcmp(el, IFL_MSG_ELEM_FIELD) ? 0 : 1);
}

int isElementValueType(const char *el)
{
    return (strcmp(el, IFL_MSG_ELEM_VALUE_TYPE) ? 0 : 1);
}

int isElementDefaultValue(const char *el)
{
    return (strcmp(el, IFL_MSG_ELEM_DEFAULT_VALUE) ? 0 : 1);
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
    if (!strcmp(type, IFL_MSG_FIELD_TYPE_V_STR)) {
        field->field.type = IFL_MSG_FIELD_TYPE_V;
    } else if (!strcmp(type, IFL_MSG_FIELD_TYPE_LV_STR)) {
        field->field.type = IFL_MSG_FIELD_TYPE_LV;
    } else if (!strcmp(type, IFL_MSG_FIELD_TYPE_TLV_STR)) {
        field->field.type = IFL_MSG_FIELD_TYPE_TLV;
    } else if (!strcmp(type, IFL_MSG_FIELD_TYPE_S_STR)) {
        field->field.type = IFL_MSG_FIELD_TYPE_S;
    } else {
        ERR("Unsupported field type=%s", type);
        return -1;
    }
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
        if (!strcmp(attr_name, IFL_MSG_FIELD_ATTR_ID)) {
            field->field.id = atoi(attr_val);
        } else if (!strcmp(attr_name, IFL_MSG_FIELD_ATTR_NAME)) {
            if (strlen(attr_val) >= sizeof(field->field.name)) {
                ERR("Insufficient memory for Name Attr len=%zu", strlen(attr_val));
                return -1;
            }
            strcpy(field->field.name, attr_val);
        } else if (!strcmp(attr_name, IFL_MSG_FIELD_ATTR_TYPE)) {
            if (IFL_ParseFieldAttrType(field, attr_val)) {
                ERR("Unsupported Type Attr val=%s", attr_val);
                return -1;
            }
        } else if (!strcmp(attr_name, IFL_MSG_FIELD_ATTR_SIZE)) {
            if (atoi(attr_val) < 0) {
                ERR("Received invalid field size=%d", atoi(attr_val));
                return -1;
            }
            field->field.size = (uint16_t)(atoi(attr_val));
        }
    }
    return 0;
}

int IFL_ParseMsgElemField(IFL_MSG_FMT_CREATOR *fmt_creator, const char *el, const char **attr)
{
    IFL_MSG_FIELD *new_field = NULL;
    IFL_MSG_FIELD *first_child;

    if (!attr) {
        ERR("Attribute is NULL for Element Field");
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
    return 0;
err:
    IFL_FreeMsgField(new_field);
    return -1;
}

/*
 * @Description: This function gets called for the start of element <ValueType>. As nothing
 * needs to be parsed here, it simply assigns the corresponding buffer to "element_data" for
 * copying element data. Callback for element data updates in this buffer. This function should
 * not get called when there is no current <Field> tag, that means <ValueType> should be sub
 * element of a <Field> tag.
 *
 * @Return: Returns Success incase of valid current field is present or else failure
 */
int IFL_ParseMsgElemValueType(IFL_MSG_FMT_CREATOR *fmt_creator, const char *el,
                                                                const char **attr)
{
    IFL_MSG_FIELD *cur = fmt_creator->cur;
    if (cur) {
        memset(cur->field.default_val_type_str, 0, sizeof(cur->field.default_val_type_str));
        fmt_creator->element_data = cur->field.default_val_type_str;
        fmt_creator->element_data_size = sizeof(cur->field.default_val_type_str);
        return 0;
    } else {
        ERR("Current node is NULL for element %s", el);
    }
    return 0;
}

/* @Description: This function gets called for the end element of <ValueType>. This parses
 * the data of the element.
 *
 * @Return: Returns Success incase of valid Data or else failure
 */
int IFL_ParseMsgElemValueTypeData(IFL_MSG_FMT_CREATOR *fmt_creator)
{
    IFL_MSG_FIELD *cur = fmt_creator->cur;
    char *val_type_str;
    if (!cur) {
        ERR("Receiving Value Type element end and current is NULL");
        return -1;
    }
    val_type_str = cur->field.default_val_type_str;
    if (!strcmp(val_type_str, IFL_MSG_FIELD_VAL_TYPE_STR_UINT)) {
        cur->field.default_val_type = IFL_MSG_FIELD_VAL_TYPE_UINT;
    } else if (!strcmp(val_type_str, IFL_MSG_FIELD_VAL_TYPE_STR_HEX)) {
        cur->field.default_val_type = IFL_MSG_FIELD_VAL_TYPE_HEX;
    } else {
        ERR("Unsupported value type=%s", val_type_str);
        return -1;
    }
    return 0;
}

/*
 * @Description: This function gets called for the start of element <DefaultValue>. As nothing
 * needs to be parsed here, it simply assigns the corresponding buffer to "element_data" for
 * copying element data. Callback for element data updates in this buffer. This function should
 * not get called when there is no current <Field> tag, that means <DefaultValue> should be sub
 * element of a <Field> tag.
 *
 * @Return: Returns Success incase of valid current field is present or else failure
 */
int IFL_ParseMsgElemDefaultValue(IFL_MSG_FMT_CREATOR *fmt_creator, const char *el,
                                                                    const char **attr)
{
    IFL_MSG_FIELD *cur = fmt_creator->cur;
    if (cur) {
        memset(cur->field.default_val_str, 0, sizeof(cur->field.default_val_str));
        fmt_creator->element_data = cur->field.default_val_str;
        fmt_creator->element_data_size = sizeof(cur->field.default_val_str);
        return 0;
    } else {
        ERR("Current node is NULL for element %s", el);
        return -1;
    }
}

/* @Description: This function gets called for the end element of <DefaultValue>. This parses
 * the data of the element.
 *
 * @Return: Returns Success incase of valid Data or else failure
 */
int IFL_ParseMsgElemDefaultValueData(IFL_MSG_FMT_CREATOR *fmt_creator)
{
    IFL_MSG_FIELD *cur = fmt_creator->cur;
    char *val_str;
    uint16_t val_type;
    int idx = 0, out_idx = 0;
    uint32_t hex_val;
    if (!cur) {
        ERR("Receiving Default Value element end and current is NULL");
        return -1;
    }
    val_str = cur->field.default_val_str;
    val_type = cur->field.default_val_type;
    if (val_type) {
        if (!strlen(val_str)) {
            ERR("Value type=%d is configured and default value is NULL str", val_type);
            return -1;
        }
        switch(val_type) {
            case IFL_MSG_FIELD_VAL_TYPE_UINT:
                sscanf(val_str, "%u", &cur->field.default_val.u32);
                TRACE("Field=%d, id=%d, default_val=%u", cur->field.name, cur->field.id,
                        cur->field.default_val.u32);
                break;
            case IFL_MSG_FIELD_VAL_TYPE_HEX:
                do {
                    sscanf(val_str + idx, "%02X", &hex_val);
                    cur->field.default_val.hex[out_idx] = hex_val & 0xFF;
                    idx += 2;
                    out_idx++;
                } while(idx < strlen(val_str));
                cur->field.default_val_size = out_idx;
                TRACE("Field=%d, id=%d", cur->field.name, cur->field.id);
                break;
            default:
                ERR("Invalid Val type %d", val_type);
                return -1;
        }
    }
    return 0;
}

int IFL_ParseMsgElem(IFL_MSG_FMT_CREATOR *fmt_creator, const char *el, const char **attr)
{
    TRACE("Start Element=%s", el);

    if (isElementField(el)) {
        if (IFL_ParseMsgElemField(fmt_creator, el, attr)) {
            ERR("Parsing Element Field failed");
            goto err;
        }
    } else if (isElementValueType(el)) {
        if (IFL_ParseMsgElemValueType(fmt_creator, el, attr)) {
            ERR("Parsing Element Value Type failed");
            goto err;
        }
    } else if (isElementDefaultValue(el)) {
        if (IFL_ParseMsgElemDefaultValue(fmt_creator, el, attr)) {
            ERR("Parsing Element Default Value failed");
            goto err;
        }
    }

    TRACE("Msg Field head=%p, cur=%p, cur_depth=%d", fmt_creator->head, fmt_creator->cur,
            (fmt_creator->cur ? fmt_creator->cur->depth : -1));
    return 0;
err:
    return -1;
}

int IFL_MsgElemStart(IFL_MSG_FMT_CREATOR *fmt_creator, const char *el, const char **attr)
{
    IFL_CHK_ERR((!el), "Element name is Null", return -1);
    if (IFL_ParseMsgElem(fmt_creator, el, attr)) {
        ERR("Parsing Msg Element %s failed", el);
        return -1;
    }
    return 0;
}

int IFL_MsgElemData(IFL_MSG_FMT_CREATOR *fmt_creator, const char *data)
{
    if ((fmt_creator->element_data) && (fmt_creator->element_data_size > strlen(data))) {
        TRACE("Updating Element data=%s", data);
        strcpy(fmt_creator->element_data, data);
        fmt_creator->element_data = NULL;
        fmt_creator->element_data_size = 0;
        return 0;
    } else {
        ERR("Element data=%s not able to copy to %s:%d", data,
                fmt_creator->element_data, fmt_creator->element_data_size);
        return -1;
    }
}

void IFL_UpdateTreeReverseDepth(IFL_MSG_FIELD *child, IFL_MSG_FIELD *parent)
{
    if (child && parent) {
        if (parent->depth < (child->depth + 1)) {
            parent->depth = child->depth + 1;
        }
    }
}

int IFL_MsgElemEnd(IFL_MSG_FMT_CREATOR *fmt_creator, const char *el)
{
    TRACE("End Element=%s", el);
    if (isElementField(el)) {
        if (fmt_creator->cur) {
            IFL_UpdateTreeReverseDepth(fmt_creator->cur, fmt_creator->cur->tree.parent);
            fmt_creator->cur = fmt_creator->cur->tree.parent;
            if ((fmt_creator->cur) && (fmt_creator->cur->depth > IFL_MAX_MSG_FIELD_DEPTH)) {
                ERR("Crossed max field depth=%u", fmt_creator->cur->depth);
                return -1;
            }
        } else {
            ERR("Abnormal state, Cur=%p, element=%s", fmt_creator->cur, el);
            return -1;
        }
    } else if (isElementValueType(el)) {
        if (IFL_ParseMsgElemValueTypeData(fmt_creator)) {
            ERR("Parsing Value Type Data failed");
            return -1;
        }
    } else if (isElementDefaultValue(el)) {
        if (IFL_ParseMsgElemDefaultValueData(fmt_creator)) {
            ERR("Parsing Default Value Data failed");
            return -1;
        }
    }
    
    TRACE("Msg Field head=%p, cur=%p, cur_depth=%d", fmt_creator->head, fmt_creator->cur,
            (fmt_creator->cur ? fmt_creator->cur->depth : -1));
    return 0;
}

