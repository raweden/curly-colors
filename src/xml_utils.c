
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "xml_utils.h"


xmlNode *
xmlNodeNextElementSibling(xmlNode *node)
{
    for (; node != NULL; node = node->next) {
        if (node->type == XML_ELEMENT_NODE) {
            return node;
        }
    }

    return NULL;
}

const char *
xmlElementGetPListKey(xmlNode *node)
{
    xmlNode *textNode;
    textNode = node->children;
    if (textNode && textNode->type == XML_TEXT_NODE && textNode->next == NULL)
        return (const char *)textNode->content;

    return NULL;
}

bool
xmlElementIsPListString(xmlNode *node)
{
    return node && node->type == XML_ELEMENT_NODE && node->name && strcmp((const char *)node->name, "string") == 0;
}

bool
xmlElementIsPListBoolean(xmlNode *node)
{
    return node && node->type == XML_ELEMENT_NODE && node->name && (strcmp((const char *)node->name, "true") == 0 || strcmp((const char *)node->name, "false") == 0);
}

bool
xmlElementGetPListBoolean(xmlNode *node)
{
    int result = -1;

    if (strcmp((const char *)node->name, "true") == 0) {
        result = true;
    } else if (strcmp((const char *)node->name, "false") == 0) {
        result = false;
    }

    assert(result != -1);

    return result;
}

char *
xmlElementCopyPListString(xmlNode *node)
{
    xmlNode *textNode;
    char *src, *dst;
    size_t srcsz;

    textNode = node->children;
    
    if (textNode && textNode->type == XML_TEXT_NODE && textNode->next == NULL) {
        src = (char *)textNode->content;
        srcsz = strlen(src);
        dst = (char *)malloc(srcsz + 1);
        memcpy(dst, src, srcsz);
        dst[srcsz] = '\0';

        return dst;
    }

    return NULL;
}

static int
xmlElementTextContentLength(xmlNode *node)
{
    xmlNode *child;
    size_t bufsz, strsz;

    bufsz = 0;

    if (node == NULL) {
        return 0;
    }

    if (node->type == XML_TEXT_NODE || node->type == XML_CDATA_SECTION_NODE) {
        strsz = strlen((const char *)node->content);
        return strsz;
    }

    child = node->children;

    for (; child != NULL; child = child->next) {
        if (child->type == XML_TEXT_NODE) {
            strsz = strlen((const char *)child->content);
            bufsz += strsz;
        } else if (child->type == XML_ELEMENT_NODE) {
            bufsz += xmlElementTextContentLength(child->children);
        }
    }

    return bufsz;
}

static void
xmlElementTextContentCopy(xmlNode *node, char **strp)
{
    xmlNode *child;
    char *buf;
    size_t strsz;

    buf = *strp;

    if (node == NULL) {
        return;
    }

    if (node->type == XML_TEXT_NODE || node->type == XML_CDATA_SECTION_NODE) {
        strsz = strlen((const char *)node->content);
        memcpy(buf, (const char *)node->content, strsz);
        buf += strsz;
        return;
    }

    child = node->children;

    for (; child != NULL; child = child->next) {
        if (child->type == XML_TEXT_NODE) {
            strsz = strlen((const char *)child->content);
            memcpy(buf, (const char *)child->content, strsz);
            buf += strsz;
        } else if (child->type == XML_ELEMENT_NODE) {
            xmlElementTextContentCopy(child->children, &buf);
        }
    }

    *strp = buf;
}

char *
xmlElementDuplicateTextContent(xmlNode *node)
{
    xmlNode *child;
    char *buf, *bufp;
    size_t srcsz, dstsz;

    srcsz = xmlElementTextContentLength(node);
    buf = malloc(srcsz + 1);
    if (buf == NULL) {
        return NULL;
    }

    bufp = buf;
    xmlElementTextContentCopy(node, &bufp);

    dstsz = bufp - buf;
    printf("copied textContent of %zu length\n", dstsz);

    *bufp = '\0';

    return buf;
}

bool
xmlElementIsPListKey(xmlNode *node)
{
    return node && node->type == XML_ELEMENT_NODE && node->name && strcmp((const char *)node->name, "key") == 0;
}

bool
xmlElementIsPListDict(xmlNode *node)
{
    return node && node->type == XML_ELEMENT_NODE && node->name && strcmp((const char *)node->name, "dict") == 0;
}

bool
xmlElementIsPListArray(xmlNode *node)
{
    return node && node->type == XML_ELEMENT_NODE && node->name && strcmp((const char *)node->name, "array") == 0;
}

int
xmlNodeTextContent(xmlNode *node, char *buf, size_t bufsz)
{
    xmlNode *child;
    char *bufp, *bufend;
    size_t srcsz;

    bufp = buf;
    bufend = bufp + bufsz;

    for (; node != NULL; node = node->next) {
        if (node->type == XML_TEXT_NODE) {
            srcsz = strlen((char *)node->content);
            if (srcsz + 1 > (bufend - bufp)) {
                *bufp = '\0';
                return -1;
            }
            memcpy(bufp, (char *)node->content, srcsz);
            bufp += srcsz;
        }
    }

    *bufp = '\0';

    return bufp - bufend;
}