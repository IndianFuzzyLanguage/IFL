#include <string.h>
#include <ctype.h>

#include "ifl_types.h"
#include "ifl_conf_parser.h"
#include "ifl_log.h"

#include "expat.h"

#define BUF_SIZE_FOR_XML_READING 1024

int g_depth = 0;
void XMLCALL start(void *data, const XML_Char *el, const XML_Char **attr)
{
    int i;

    for (i = 0; i < g_depth; i++) {
        printf("  ");
    }

    printf("%s", el);
    if (data) {
        printf("=%s", (char *)data);
    }
    for (i = 0; attr[i]; i += 2) {
        printf(" %s=%s", attr[i], attr[i + 1]);
    }
    printf("\n");
    g_depth++;
}

void XMLCALL end(void *data, const XML_Char *el)
{
    (void)data;
    (void)el;
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

void IFL_ParseConf(const char *xml_file_name, const char *xml_content)
{
    char buf[BUF_SIZE_FOR_XML_READING] = {0};
    XML_Parser xparser;
    FILE *xml_file;
    int done;
    int len;

    if (!xml_file_name) {
        ERR("No XML file\n");
        goto err;
    }
    xml_file = fopen(xml_file_name, "r");
    if (!xml_file) {
        ERR("XML File open failed\n");
        goto err;
    }

    xparser = XML_ParserCreate(NULL);
    if (!xparser) {
        ERR("XML Parser creation failed\n");
        goto err;
    }

    XML_SetElementHandler(xparser, start, end);
    XML_SetCharacterDataHandler(xparser, data_handler);

    do {
        len = (int)fread(buf, 1, sizeof(buf) - 1, xml_file);
        if (len <= 0) {
            ERR("fread ret=%d\n", len);
            goto err;
        }
        done = feof(xml_file);
        if (XML_Parse(xparser, buf, len, done) == XML_STATUS_ERROR) {
            ERR("Parsing XML failed\n");
            goto err;
        }
    } while (!done);
    XML_ParserFree(xparser);
err:
    return;
}

