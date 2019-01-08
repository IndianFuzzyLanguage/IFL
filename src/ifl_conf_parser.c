#include <string.h>
#include <ctype.h>

#include "ifl_types.h"
#include "ifl_conf_parser.h"
#include "ifl_msg_format.h"
#include "ifl_log.h"

#include "expat.h"

#define BUF_SIZE_FOR_XML_READING 1024
#define MAX_ELEMENT_SIZE 256

int g_depth = 0;

char *get_element_str(char *out, uint16_t out_len, const XML_Char *el, const XML_Char **attr)
{
    int idx;
    int ret;
    int i;
    ret = snprintf(out, out_len, "%s", el);
    if ((ret < 0) || (ret >= out_len)) {
        ERR("snprintf failed for tag[%s]", el);
        ret = 0;
    }
    idx = ret;
    for (i = 0; attr[i]; i += 2) {
        ret = snprintf(out + idx, out_len - idx, "%s", el);
        if ((ret < 0) || (ret >= out_len)) {
            ERR("snprintf failed for attr[%s=%s]", attr[i], attr[i + 1]);
            ret = 0;
        }
        idx += ret;
    }
    return out;
}

void XMLCALL start_handler(void *data, const XML_Char *el, const XML_Char **attr)
{
    char element[MAX_ELEMENT_SIZE] = {0};
    IFL_MSG_FMT_CREATOR *app_data = (IFL_MSG_FMT_CREATOR *)data;
    int i;

    for (i = 0; i < g_depth; i++) {
        printf("  ");
    }

    if (app_data) {
        if (IFL_MsgElementStart(app_data, el, attr)) {
            ERR("Failed field [%s]", get_element_str(element, sizeof(element), el, attr));
        }
    }
    printf("%s\n", get_element_str(element, sizeof(element), el, attr));
    g_depth++;
}

void XMLCALL end_handler(void *data, const XML_Char *el)
{
    IFL_MSG_FMT_CREATOR *app_data = (IFL_MSG_FMT_CREATOR *)data;
    (void)data;
    (void)el;
    if (app_data) {
        if (IFL_MsgElementEnd(app_data, el)) {
            ERR("End Field update failed");
        }
    }
    g_depth--;
}

char *trim_space(char *in)
{
    char *out = NULL;
    int len;
    if (in) {
        len = strlen(in);
        while(len && isspace(in[len - 1])) --len;
        while(len && *in && isspace(*in)) ++in, --len;
        if (len) {
            out = strndup(in, len);
        }
    }
    return out;
}

void XMLCALL data_handler(void *data, const XML_Char *buf, int len)
{
    char *buf_new;
    char *buf_new2;
    if (buf && (len > 1)) {
        buf_new = calloc(1, len+1);
        if (buf_new) {
            memcpy(buf_new, buf, len);
            buf_new2 = trim_space(buf_new);
            free(buf_new);
            buf_new = NULL;
            if (buf_new2) {
                printf(" [%s]\n", buf_new2);
                free(buf_new2);
            }
        }
    }
}

IFL_MSG_FIELD *IFL_ParseConf(const char *xml_file_name, const char *xml_content)
{
    char buf[BUF_SIZE_FOR_XML_READING] = {0};
    XML_Parser xparser;
    IFL_MSG_FMT_CREATOR app_data = {0};
    FILE *xml_file;
    int done;
    int len;

    if (!xml_file_name) {
        ERR("No XML file");
        goto err;
    }
    xml_file = fopen(xml_file_name, "r");
    if (!xml_file) {
        ERR("XML File open failed");
        goto err;
    }

    xparser = XML_ParserCreate(NULL);
    if (!xparser) {
        ERR("XML Parser creation failed");
        goto err;
    }

    XML_SetElementHandler(xparser, start_handler, end_handler);
    XML_SetCharacterDataHandler(xparser, data_handler);
    XML_SetUserData(xparser, &app_data);

    do {
        len = (int)fread(buf, 1, sizeof(buf) - 1, xml_file);
        if (len <= 0) {
            ERR("fread ret=%d", len);
            goto err;
        }
        done = feof(xml_file);
        if (XML_Parse(xparser, buf, len, done) == XML_STATUS_ERROR) {
            ERR("Parsing XML failed");
            goto err;
        }
    } while (!done);
    XML_ParserFree(xparser);
    return app_data.head;
err:
    return NULL;
}

