#include <string.h>

#include "ifl_types.h"
#include "ifl_msg_format.h"
#include "ifl_conf_parser.h"
#include "ifl_util.h"
#include "ifl_log.h"

#define FIELD_ELEMENT "Field"
#define TAG_FIELD_ELEMENT "TagField"
#define LENGTH_FIELD_ELEMENT "LengthField"
#define VALUE_FIELD_ELEMENT "ValueField"

#define FIELD_ATTR_ID "id"
#define FIELD_ATTR_NAME "name"
#define FIELD_ATTR_TYPE "type"
#define FIELD_ATTR_SIZE "size"

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
            ERR("Field stack push failed\n");
            return NULL;
        }
    }
    return cur;
}

void IFL_LogMsgFormat(IFL_MSG_FIELD *msg, uint8_t log_level)
{
    IFL_MSG_FIELD *cur;
    IFL_FIELD_STACK *stack;
    stack = IFL_InitFieldStack(msg);
    LOG(log_level, "Msg tree with max depth=%u", msg->depth);
    while ((stack) && (cur = IFL_GetNextField(msg, stack))) {
        LOG(log_level, "Cur=%p, id=%d, name=%s, size=%d, depth=%d",
                cur, cur->field.id, cur->field.name, cur->field.size, cur->depth);
    }
    IFL_FiniFieldStack(stack);
}

int isElementField(const char *el)
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

int IFL_ParseMsgElementField(IFL_MSG_FMT_CREATOR *fmt_creator, const char *el, const char **attr)
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

int IFL_ParseMsgElement(IFL_MSG_FMT_CREATOR *fmt_creator, const char *el, const char **attr)
{
    TRACE("Start Element=%s", el);

    if (isElementField(el)) {
        if (IFL_ParseMsgElementField(fmt_creator, el, attr)) {
            ERR("Parsing Element Field failed");
            goto err;
        }
    }

    TRACE("Msg Field head=%p, cur=%p, cur_depth=%d", fmt_creator->head, fmt_creator->cur,
            (fmt_creator->cur ? fmt_creator->cur->depth : -1));
    return 0;
err:
    return -1;
}

int IFL_MsgElementStart(IFL_MSG_FMT_CREATOR *fmt_creator, const char *el, const char **attr)
{
    IFL_CHK_ERR((!el), "Element name is Null", return -1);
    TRACE("Start Element=%s", el);
    if (IFL_ParseMsgElement(fmt_creator, el, attr)) {
        ERR("Parsing Msg Element %s failed", el);
        return -1;
    }
    return 0;
}

void IFL_UpdateTreeReverseDepth(IFL_MSG_FIELD *child, IFL_MSG_FIELD *parent)
{
    if (child && parent) {
        if (parent->depth < (child->depth + 1)) {
            parent->depth = child->depth + 1;
        }
    }
}

int IFL_MsgElementEnd(IFL_MSG_FMT_CREATOR *fmt_creator, const char *el)
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
    }
    
    TRACE("Msg Field head=%p, cur=%p, cur_depth=%d", fmt_creator->head, fmt_creator->cur,
            (fmt_creator->cur ? fmt_creator->cur->depth : -1));
    return 0;
}

